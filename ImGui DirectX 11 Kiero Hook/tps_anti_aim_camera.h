#pragma once
// ============================================================================
// tps_anti_aim_camera.h
//
// Isolated, self-contained third-person anti-aim camera workaround.
//
// This file NEVER touches InputFilter, AimingData, CreateMove/BuildCommand,
// or any network snapshot (AimSnapshot/MovementSnapshot/PlayerSnapshot/mwq).
// Every previous regression (vertical lock, "up" snap, left/right-only
// control) came from writing into AimingData shared by those systems. This
// file only ever writes Camera.main's Transform, and only while Third Person
// + anti-aim are both enabled.
//
// It is removable in one step: delete the #include in main.cpp and the two
// call sites (hook install + hook body). Nothing else depends on it.
//
// Ground truth (dump1/dump.cs): CameraController is a MonoBehaviour attached
// to the active scene camera (static clgq/clgr instances, own Camera field
// clgs at +0x28). Its OnPreCull (RVA 0xB74D50) runs after every Update/
// LateUpdate for the frame, right before that camera culls/renders. That is
// the latest point at which the true FPS/TPS aim state (AimingData, fake or
// real) has already been fully consumed by native camera code -- so writing
// Camera.main's Transform here is the last write before the frame is drawn.
//
// Previous attempt (commit 8f6d309) used this exact same hook and the same
// condition and still showed the camera stuck looking up. That was never
// root-caused with real data -- only guessed at blind. This version adds
// counters and a status string (rendered in the Anti-Aim tab) so the next
// in-game test tells us definitively:
//   - fires == 0            -> the hook never installed / never got called.
//   - fires > 0, applied==0 -> condition false every frame (see status text
//                              for which check failed -- most likely this
//                              CameraController instance never equals
//                              Camera.main, meaning OnPreCull here belongs to
//                              a different camera than the one actually
//                              rendering the player view).
//   - applied > 0 but still visibly wrong -> the write itself is happening,
//                              but something else (another native camera
//                              writer, or a parent Transform) overrides it
//                              between here and the GPU frame, or FPSCamera/
//                              TPS camera position (not just rotation) needs
//                              correcting too.
// Any of those three outcomes point to a different, exact next step instead
// of another blind guess.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <windows.h>

// Forward declarations only. Definitions live in main.cpp; this header must
// never redefine or duplicate any anti-aim/network/input state.
extern bool thirdPersonEnabled;
extern bool silentAntiAimEnabled;
extern bool silentAntiAimRealCameraValid;
extern Vector3 silentAntiAimRealCameraAngles;
extern uintptr_t(__fastcall* o_Camera_get_main)();
extern uintptr_t(__fastcall* o_Component_get_transform)(uintptr_t);
extern void(__fastcall* o_Transform_set_eulerAngles)(uintptr_t, Vector3);
extern volatile LONG silentAntiAimCameraRestoreCalls;

volatile LONG g_TpsCameraHookFires = 0;
volatile LONG g_TpsCameraTargetFires = 0;
volatile LONG g_TpsCameraRestoreApplied = 0;
char g_TpsCameraWorkaroundStatus[160] = "Idle";

// Called once per frame from the CameraController.OnPreCull hook in
// main.cpp, strictly after the native OnPreCull has already run for this
// specific camera instance. controlledCamera is that camera's own Camera
// component (dump1 CameraController::clgs, +0x28), read by the caller.
// isFpsCamera is dump1 CameraController::_isFpsCamera at +0x22. Camera.main
// remains the tagged FPS camera in TPS mode, so it must NOT be used to select
// the third-person camera.
void TpsCameraWorkaround_AfterNativeOnPreCull(
    uintptr_t controlledCamera, bool isFpsCamera)
{
    InterlockedIncrement(&g_TpsCameraHookFires);

    if (!thirdPersonEnabled || !silentAntiAimEnabled) {
        strcpy_s(g_TpsCameraWorkaroundStatus, "Idle: TPS or anti-aim disabled");
        return;
    }
    // dump1 has two CameraController singletons. In third person Camera.main
    // still resolves to the tagged FPS camera, while the visible orbit camera
    // is the controller whose serialized _isFpsCamera flag is false.
    if (isFpsCamera) return;
    InterlockedIncrement(&g_TpsCameraTargetFires);

    if (!silentAntiAimRealCameraValid) {
        strcpy_s(g_TpsCameraWorkaroundStatus, "TPS found; waiting for real angles");
        return;
    }
    if (!controlledCamera || !o_Component_get_transform ||
        !o_Transform_set_eulerAngles) {
        strcpy_s(g_TpsCameraWorkaroundStatus, "TPS found; required pointer missing");
        return;
    }

    __try {
        const uintptr_t cameraTransform =
            o_Component_get_transform(controlledCamera);
        if (!cameraTransform) {
            strcpy_s(g_TpsCameraWorkaroundStatus, "TPS found; no camera Transform");
            return;
        }
        o_Transform_set_eulerAngles(cameraTransform,
            silentAntiAimRealCameraAngles);
        InterlockedIncrement(&g_TpsCameraRestoreApplied);
        InterlockedIncrement(&silentAntiAimCameraRestoreCalls);
        strcpy_s(g_TpsCameraWorkaroundStatus,
            "Applied to non-FPS third-person camera");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(g_TpsCameraWorkaroundStatus,
            "Exception while applying to TPS camera");
    }
}
