#pragma once
#include <vector>
#include <jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <init/Dobby/dobby.h>
#include "globals.hpp"

struct UnityEngine_Touch_Fields {
    int32_t m_FingerId;
    Vector2 m_Position;
    Vector2 m_RawPosition;
    Vector2 m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

struct PlayerInputs {
    uint8_t pad[0x8];
    float Horizontal;
    float Vertical;
    bool IsCrouching;
    bool IsToReload;
    uint8_t pad2[0x2];
    int SwitchToWeapon;
    bool IsToAim;
    bool IsToFire;
    bool IsToJump;
    bool IsToDrop;
    bool IsToAction;
    bool IsToInspect;
    bool IsToPickup;
    uint8_t pad3[0x1];
    Vector3 AimEulerAngles;
    Vector2 DeltaAimAngles;
};

template<typename T>
struct blended_value_t {
    uint8_t pad[0x8];
    T actual;
    T actualPrevFrame;
    T blended;
    T previous;
    float timeFix;
    float blendDuration;
};

class hooks {
public:
    static EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
    static void update(void* player);
    static void lateupdate(void* i);
    static bool raycast(void* scene, Ray* ray, float dist, void* hit, int lm, int interact);
    static bool get_isGrounded(void* instance);
    static void OnOcclusionBecameInvisible(void* instance);
    static int get_touchCount();
    static void InputFilter(void* thisptr, PlayerInputs* inputs);
    
    static inline EGLBoolean (*old_eglSwapBuffers)(EGLDisplay, EGLSurface);
    static inline void (*old_update)(void*);
    static inline void (*old_lateupdate)(void*);
    static inline bool (*o_raycast)(void*, Ray*, float, void*, int, int);
    static inline bool (*old_get_isGrounded)(void*);
    static inline void (*old_OnOcclusionBecameInvisible)(void*);
    static inline int (*old_input_get_touchCount)();
    static inline void (*og_InputFilter)(void*, PlayerInputs*);
    static inline PlayerInputs oldinputs;
    
    static inline bool setup;
};
