#include "ragebot.hpp"
#include "globals.hpp"
#include "hooks.hpp"
#include <init/Dobby/dobby.h>

static void force_move(float horizontal, float vertical) {
    if (!me) return;
    void* mov = *(void**)((uint64_t)me + 0x50);
    if (!mov) return;
    void* td = *(void**)((uint64_t)mov + 0x44);
    if (!td) return;
    auto* dir = *(blended_value_t<Vector2>**)((uint64_t)td + 0x94);
    if (!dir) return;
    dir->actual = Vector2(horizontal, vertical);
}

static void movement_fix(float orig_yaw, float antiaim_yaw, PlayerInputs* inputs) {
    if (!inputs || !me) return;
    if (inputs->Vertical == 0.f && inputs->Horizontal == 0.f) return;

    auto deg2rad = [](float deg) { return deg * M_PI / 180.f; };
    auto normalize_yaw = [](float yaw) { 
        while (yaw < 0) yaw += 360.f;
        while (yaw >= 360.f) yaw -= 360.f;
        return yaw;
    };

    float f1 = normalize_yaw(orig_yaw);
    float f2 = normalize_yaw(antiaim_yaw);
    float yaw_delta = (f2 < f1) ? std::abs(f2 - f1) : 360.f - std::abs(f1 - f2);
    yaw_delta = 360.f - yaw_delta;

    float forward = inputs->Vertical;
    float side = inputs->Horizontal;

    inputs->Horizontal = std::cos(deg2rad(yaw_delta)) * side + std::cos(deg2rad(yaw_delta + 90)) * forward;
    inputs->Vertical = std::sin(deg2rad(yaw_delta)) * side + std::sin(deg2rad(yaw_delta + 90)) * forward;
    force_move(inputs->Horizontal, inputs->Vertical);
}

void ragebot::silentaim::run(void* scene, Ray* ray) {
    if (!ragebot::aim::silent) return;
    
    float minA = ragebot::aim::fov;
    Vector3 bestDir, hPos;
    bool found = false;
    
    for (void* p : players) {
        if (!p) continue;
        void* bmp = get_bipedmap(p);
        if (!bmp) continue;
        
        static auto headOffset = BNM::LoadClass("Axlebolt.Standoff.Player", "BipedMap").GetFieldByName("Head").GetOffset();
        void* head = *(void**)((uint64_t)bmp + headOffset);
        if (!head) continue;
        
        hPos = get_position(head);
        float ang = Vector3::Angle(hPos - ray->m_Origin, ray->m_Direction);
        
        if (ang <= minA && hPos.x != 0) {
            minA = ang;
            bestDir = Vector3::Normalize(hPos - ray->m_Origin);
            found = true;
        }
    }
    
    if (found) ray->m_Direction = bestDir;
}

void ragebot::antiaim::run(PlayerInputs* inputs) {
    if (!me || !inputs) return;
    
    void* aim_controller = *(void**)((uint64_t)me + BNM::LoadClass("Axlebolt.Standoff.Player", "PlayerController").GetFieldByName("<AimController>k__BackingField").GetOffset());
    if (!aim_controller) return;
    
    void* aim_data = *(void**)((uint64_t)aim_controller + BNM::LoadClass("Axlebolt.Standoff.Player.Aim", "AimController").GetFieldByName("aimingData").GetOffset());
    if (!aim_data) return;
    
    Vector3* curAimAngle = (Vector3*)((uint64_t)aim_data + BNM::LoadClass("Axlebolt.Standoff.Player.Aim", "AimingData").GetFieldByName("curAimAngle").GetOffset());
    Vector3* curEulerAngles = (Vector3*)((uint64_t)aim_data + BNM::LoadClass("Axlebolt.Standoff.Player.Aim", "AimingData").GetFieldByName("curEulerAngles").GetOffset());
    
    if (orig_angles.x == 0.f && orig_angles.y == 0.f)
        orig_angles = Vector3(curAimAngle->x, curEulerAngles->y, 0.f);
    
    orig_angles.x -= inputs->DeltaAimAngles.x * 2.f;
    orig_angles.y += inputs->DeltaAimAngles.y * 2.f;
    
    if (!enable) {
        orig_angles = Vector3(0.f, 0.f, 0.f);
        return;
    }
    
    float p_out = 0.f;
    if (pitch_mode == 0) p_out = 90.f;
    else if (pitch_mode == 1) p_out = -90.f;
    else if (pitch_mode == 2) p_out = (float)(rand() % 181 - 90);
    
    float y_out = orig_angles.y + 180.f;
    
    if (yaw_mode == 1) {
        static int tick = 0;
        static bool side = false;
        if (++tick >= jitter_ticks) { tick = 0; side = !side; }
        y_out += side ? 35.f : -35.f;
    } else if (yaw_mode == 2) {
        static float spin = 0.f;
        spin += spin_speed;
        if (spin > 360.f) spin -= 360.f;
        y_out = spin;
    }
    
    bool stop = inputs->IsToFire || inputs->IsToDrop || inputs->IsToPickup;
    
    if (!stop) {
        curAimAngle->x = p_out;
        curEulerAngles->y = y_out;
        movement_fix(orig_angles.y, y_out, inputs);
    } else {
        curAimAngle->x = orig_angles.x;
        curEulerAngles->y = orig_angles.y;
        movement_fix(orig_angles.y, orig_angles.y, inputs);
    }
}

void ragebot::antiaim::init() {
    static auto get_Instance = (void* (*)()) BNM::LoadClass("Axlebolt.Standoff.Controls", "PlayerControls").GetMethodByName("get_Instance", 0).GetOffset();
    static BNM::Field<void*> inputFilterField = BNM::LoadClass("Axlebolt.Standoff.Controls", "PlayerControls").GetFieldByName("<InputFilter>k__BackingField");
    
    void* controls = get_Instance();
    if (!controls) return;
    
    inputFilterField.setInstance((BNM::IL2CPP::Il2CppObject*)controls);
    void* filter = inputFilterField.get();
    if (!filter) return;
    
    auto delegate = (BNM::IL2CPP::Il2CppDelegate*)filter;
    if (!delegate->method_ptr) return;
    
    DobbyHook((void*)delegate->method_ptr, (void*)hooks::InputFilter, (void**)&hooks::og_InputFilter);
}
