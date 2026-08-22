#pragma once
// Third-person camera is intentionally isolated from anti-aim input/network.
// Exactly two public functions:
//   1) TpsThirdPerson_SetViewState() - native FPS/TPS transition.
//   2) TpsThirdPerson_LateUpdate()   - late camera placement.
//
// Port of the supplied CMisc::thirdperson/lateUpdate design. It never writes
// InputFilter, AimingData, CreateMove, AimSnapshot, MovementSnapshot or
// PlayerSnapshot. The camera is placed explicitly from local-player origin
// and the REAL local view angles, so fake anti-aim pitch/yaw cannot drag it.

#include <cstdint>
#include <cstring>
#include <cmath>
#include <windows.h>

extern bool thirdPersonEnabled;
extern bool silentAntiAimEnabled;
extern bool silentAntiAimRealCameraValid;
extern Vector3 silentAntiAimRealCameraAngles;
extern uintptr_t liveHudLocalPlayer;
extern float thirdPersonHorizontalOffset;
extern float thirdPersonHeightAdjustment;
extern float thirdPersonDistanceAdjustment;
extern volatile LONG pendingThirdPersonCommand;
extern char thirdPersonStatus[96];

extern uintptr_t(__fastcall* o_Component_get_transform)(uintptr_t);
extern Vector3(__fastcall* o_Transform_get_position)(uintptr_t);
extern void(__fastcall* o_Transform_set_position)(uintptr_t, Vector3);
extern void(__fastcall* o_Transform_set_eulerAngles)(uintptr_t, Vector3);
extern Il2CppArray*(__fastcall* o_Physics_RaycastAll)(
    Vector3, Vector3, float, int, int, const Il2CppMethod*);
extern uintptr_t(__fastcall* o_RaycastHit_get_collider)(
    RaycastHitNative*, const Il2CppMethod*);
extern uintptr_t(__fastcall* o_Component_GetInParent)(
    uintptr_t, uintptr_t, bool, const Il2CppMethod*);
extern Il2CppObject* g_PlayerControllerReflectionType;

// Existing native transition is kept as the authoritative view-mode switch.
// Declared here because its implementation and runtime ownership stay in
// main.cpp; this isolated module only sequences it.
static bool ApplyNativeThirdPersonState();

volatile LONG g_TpsCameraLateUpdates = 0;
char g_TpsCameraStatus[160] = "Idle";

// Function 1: switch native view mode. Call only for a queued toggle/config
// change, not every frame (mirrors source's updateView guard).
static bool TpsThirdPerson_SetViewState()
{
    const bool applied = ApplyNativeThirdPersonState();
    if (applied) {
        strcpy_s(g_TpsCameraStatus, thirdPersonEnabled ?
            "TPS view active; waiting for late camera" : "FPS view restored");
    }
    return applied;
}

// Function 2: source-style lateUpdate. Native camera code has already run;
// explicitly place the non-FPS camera behind the local player:
// origin + height/right offset - realForward * distance.
static void TpsThirdPerson_LateUpdate(
    uintptr_t controlledCamera, bool isFpsCamera)
{
    if (!thirdPersonEnabled || isFpsCamera) return;
    if (!liveHudLocalPlayer || !controlledCamera ||
        !o_Component_get_transform || !o_Transform_get_position ||
        !o_Transform_set_position || !o_Transform_set_eulerAngles) {
        strcpy_s(g_TpsCameraStatus, "Waiting for TPS camera/player");
        return;
    }

    __try {
        const uintptr_t playerTransform =
            o_Component_get_transform(liveHudLocalPlayer);
        const uintptr_t cameraTransform =
            o_Component_get_transform(controlledCamera);
        if (!playerTransform || !cameraTransform) {
            strcpy_s(g_TpsCameraStatus, "Waiting for camera transforms");
            return;
        }

        const Vector3 origin = o_Transform_get_position(playerTransform);
        if (!isfinite(origin.x) || !isfinite(origin.y) || !isfinite(origin.z))
            return;

        // Preserve normal control when AA is off. With AA on, use the real
        // view accumulated by InputFilter, never the fake live AimingData.
        Vector3 view = silentAntiAimRealCameraAngles;
        if (!silentAntiAimEnabled || !silentAntiAimRealCameraValid) {
            strcpy_s(g_TpsCameraStatus, "TPS active; waiting for real view");
            return;
        }

        const float pitch = view.x * 0.01745329251994329577f;
        const float yaw = view.y * 0.01745329251994329577f;
        const float cosPitch = cosf(pitch);
        const Vector3 forward(
            sinf(yaw) * cosPitch,
            -sinf(pitch),
            cosf(yaw) * cosPitch);
        const Vector3 right(cosf(yaw), 0.0f, -sinf(yaw));

        // Source default distance/currentOffset adapted to existing sliders.
        const float distance = fmaxf(0.25f,
            3.0f - thirdPersonDistanceAdjustment);
        const float height = 1.6f + thirdPersonHeightAdjustment;
        const Vector3 pivot(
            origin.x, origin.y + height, origin.z);
        Vector3 position(
            pivot.x + right.x * thirdPersonHorizontalOffset - forward.x * distance,
            pivot.y - forward.y * distance,
            pivot.z + right.z * thirdPersonHorizontalOffset - forward.z * distance);

        // Camera collision only: trace from the player pivot to the requested
        // orbit position and pull the camera in front of the nearest wall. Ignore
        // the local player's own collider. This prevents the view/crosshair from
        // sitting behind geometry while native bullets still start at the weapon.
        const float rayX = position.x - pivot.x;
        const float rayY = position.y - pivot.y;
        const float rayZ = position.z - pivot.z;
        const float rayLength = sqrtf(
            rayX * rayX + rayY * rayY + rayZ * rayZ);
        bool cameraClipped = false;
        if (isfinite(rayLength) && rayLength > 0.05f &&
            o_Physics_RaycastAll && o_RaycastHit_get_collider &&
            o_Component_GetInParent && g_PlayerControllerReflectionType) {
            const Vector3 rayDirection(
                rayX / rayLength, rayY / rayLength, rayZ / rayLength);
            Il2CppArray* hits = o_Physics_RaycastAll(
                pivot, rayDirection, rayLength, -1, 1, nullptr);
            const uintptr_t array = reinterpret_cast<uintptr_t>(hits);
            const size_t count = array ?
                *reinterpret_cast<size_t*>(array + 0x18) : 0;
            float nearestDistance = rayLength + 0.001f;
            if (array && count <= 128) {
                for (size_t i = 0; i < count; ++i) {
                    RaycastHitNative* hit = reinterpret_cast<RaycastHitNative*>(
                        array + 0x20 + i * sizeof(RaycastHitNative));
                    if (!isfinite(hit->distance) || hit->distance < 0.05f ||
                        hit->distance >= nearestDistance)
                        continue;
                    const uintptr_t collider =
                        o_RaycastHit_get_collider(hit, nullptr);
                    if (!collider) continue;
                    const uintptr_t hitPlayer = o_Component_GetInParent(
                        collider,
                        reinterpret_cast<uintptr_t>(g_PlayerControllerReflectionType),
                        true, nullptr);
                    if (hitPlayer == liveHudLocalPlayer) continue;
                    nearestDistance = hit->distance;
                }
            }
            if (nearestDistance <= rayLength) {
                const float safeDistance = fmaxf(0.05f,
                    nearestDistance - 0.15f);
                position = Vector3(
                    pivot.x + rayDirection.x * safeDistance,
                    pivot.y + rayDirection.y * safeDistance,
                    pivot.z + rayDirection.z * safeDistance);
                cameraClipped = true;
            }
        }

        o_Transform_set_eulerAngles(cameraTransform, view);
        o_Transform_set_position(cameraTransform, position);
        InterlockedIncrement(&g_TpsCameraLateUpdates);
        strcpy_s(g_TpsCameraStatus, cameraClipped ?
            "TPS camera pulled in by wall" :
            "Late camera placed from real view");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(g_TpsCameraStatus, "TPS late camera exception");
    }
}
