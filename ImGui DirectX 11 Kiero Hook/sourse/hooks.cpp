#include "hooks.hpp"
#include "features/esp.hpp"
#include "features/ragebot.hpp"
#include <gui/gui.hpp>
#include <gui/font.hpp>
#include <init/ImGui/ImGui.h>
#include <init/ImGui/ImGui_internal.h>
#include <init/ImGui/Backends/ImGui_impl_opengl3.h>

EGLBoolean hooks::eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glwidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glheight);
    
    if (!setup) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2((float)glwidth, (float)glheight);
        ImGui_ImplOpenGL3_Init("#version 100");
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScrollbarSize = 3.0f;
        static ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void*)font_binary, sizeof(font_binary), 26.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        io.Fonts->AddFontFromMemoryTTF((void*)icons_binary, sizeof(icons_binary), 30.f, &font_cfg);
        io.Fonts->AddFontFromMemoryTTF((void*)font_bold_binary, sizeof(font_bold_binary), 40.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        ImGui::GetStyle().ScaleAllSizes(2.0f);
        setup = true;
    }
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    
    if (ragebot::aim::silent) {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(glwidth / 2, glheight / 2), float(ragebot::aim::fov) * 1.1, ImColor(255, 255, 255), 100, 1.5f);
    }
    
    esp_run();
    initgui();
    
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    return old_eglSwapBuffers(dpy, surface);
}

void hooks::update(void* player) {
    if (player == NULL) return;
    
    if (IsLocal(get_photon(player))) {
        me = player;
    }
    
    if (thirdperson) {
        SetTPSView(me);
    } else {
        SetFPSView(me);
    }
    
    if (movement::speedhack) {
        void* mov = *(void**)((uint64_t)me + 0x50);
        if (mov) {
            void* tp = *(void**)((uint64_t)mov + 0x40);
            if (tp) {
                *(float*)((uint64_t)tp + 0x8) = movement::speedfloat;
            }
        }
    }
    
    if (movement::bhop) {
        void* mov = *(void**)((uint64_t)me + 0x50);
        if (mov) {
            void* tp = *(void**)((uint64_t)mov + 0x40);
            if (tp) {
                void* jmp = *(void**)((uint64_t)tp + 0x20);
                if (jmp) {
                    *(float*)((uint64_t)jmp + 0x30) = movement::bhopfloat;
                    void* td = *(void**)((uint64_t)mov + 0x44);
                    if (td) {
                        *(float*)((uint64_t)td + 0x9C) = 0.0f;
                        *(float*)((uint64_t)td + 0xA4) = 1.0f;
                        *(float*)((uint64_t)td + 0x10) = 1.0f;
                    }
                }
            }
        }
    }
    
    if (!playerFind(player)) {
        players.push_back(player);
        clearList();
    }
    
    old_update(player);
}

void hooks::lateupdate(void* i) {
    if (!og_InputFilter) ragebot::antiaim::init();
    
    if (me && ragebot::antiaim::enable) {
        set_euler(get_transform(get_camera()), ragebot::antiaim::orig_angles);
    }
    
    old_lateupdate(i);
    
    if (me) {
        thirdlerp = ImLerp(thirdlerp, thirdperson ? thirdfloat : 0.f, 0.1f);
        if (thirdlerp > 0.01f) {
            set_position(get_transform(get_camera()), (get_position(get_transform(me)) + Vector3(0, 1.5f, 0)) - get_forward(get_transform(get_camera())) * thirdlerp);
        }
    }
}

bool hooks::raycast(void* scene, Ray* ray, float dist, void* hit, int lm, int interact) {
    ragebot::silentaim::run(scene, ray);
    return o_raycast(scene, ray, dist, hit, lm, interact);
}

bool hooks::get_isGrounded(void* instance) {
    if (instance != nullptr && exploits::airjump) {
        return true;
    }
    return old_get_isGrounded(instance);
}

void hooks::OnOcclusionBecameInvisible(void* instance) {
    return;
}

int hooks::get_touchCount() {
    static void* GetTouchPtr = (void*)BNM::LoadClass("UnityEngine", "Input").GetMethodByName("GetTouch", 1).GetOffset();
    ImGuiIO& io = ImGui::GetIO();
    int touchCount = old_input_get_touchCount();
    
    if (GetTouchPtr && touchCount > 0) {
        UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields (*)(int))GetTouchPtr)(0);
        float reverseY = io.DisplaySize.y - touch.m_Position.y;
        
        switch (touch.m_Phase) {
            case 0:
            case 2:
                io.MousePos = ImVec2(touch.m_Position.x, reverseY);
                io.MouseDown[0] = true;
                break;
            case 3:
            case 4:
                io.MouseDown[0] = false;
                break;
            case 1:
                io.MousePos = ImVec2(touch.m_Position.x, reverseY);
                break;
        }
    }
    
    if (io.WantCaptureMouse) return 0;
    return touchCount;
}

void hooks::InputFilter(void* thisptr, PlayerInputs* inputs) {
    if (inputs) oldinputs = *inputs;
    ragebot::antiaim::run(inputs);
    og_InputFilter(thisptr, inputs);
}
