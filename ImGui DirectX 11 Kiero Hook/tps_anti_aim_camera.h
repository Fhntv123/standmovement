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
        const Vector3 position(
            origin.x + right.x * thirdPersonHorizontalOffset - forward.x * distance,
            origin.y + height - forward.y * distance,
            origin.z + right.z * thirdPersonHorizontalOffset - forward.z * distance);

        o_Transform_set_eulerAngles(cameraTransform, view);
        o_Transform_set_position(cameraTransform, position);
        InterlockedIncrement(&g_TpsCameraLateUpdates);
        strcpy_s(g_TpsCameraStatus, "Late camera placed from real view");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(g_TpsCameraStatus, "TPS late camera exception");
    }
}
