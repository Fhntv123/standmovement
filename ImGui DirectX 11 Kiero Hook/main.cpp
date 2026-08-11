#include "includes.h"
#include "il2cpp.h"

#include <vector>

#include <string>

#include <urlmon.h>

#include <wininet.h>

#include <wincodec.h>

#include <comdef.h>

#pragma comment(lib, "urlmon.lib")

#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "windowscodecs.lib")



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



Present oPresent;

HWND window = NULL;

WNDPROC oWndProc;

ID3D11Device* pDevice = NULL;

ID3D11DeviceContext* pContext = NULL;

ID3D11RenderTargetView* mainRenderTargetView;



bool init = false;

bool keyValidated = false;

bool unloadRequested = false;



struct Vector3 {

    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}

    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float Distance(const Vector3& v) const {

        float dx = x - v.x, dy = y - v.y, dz = z - v.z;

        return sqrt(dx * dx + dy * dy + dz * dz);

    }

    float Length() const {

        return sqrt(x * x + y * y + z * z);

    }

    float Length2D() const { return sqrt(x * x + z * z); }

    Vector3 operator-(const Vector3& v) const {

        return Vector3(x - v.x, y - v.y, z - v.z);

    }

};



struct Vector2 {

    float x, y;

    Vector2() : x(0), y(0) {}

    Vector2(float _x, float _y) : x(_x), y(_y) {}

};



struct Vector4 {

    float x, y, z, w;

};



struct Matrix16 {

    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;

};



// CharacterController methods

#define OFFSET_CHARACTERCONTROLLER_GET_ISGROUNDED  0x2F4DEB0

#define OFFSET_CHARACTERCONTROLLER_MOVE            0x2F4DD80

#define OFFSET_CHARACTERCONTROLLER_GET_VELOCITY    0x2F4DF80

#define OFFSET_TRANSFORM_GET_POSITION              0x2F06CE0

#define OFFSET_GET_PLAYERCONTROLLER                0x83F820  // PlayerManager.qjm() -> dcvh (current player controller)

#define OFFSET_CAMERA_MAIN                         0x2EC1090

#define OFFSET_WORLDTOSCREENPOINT                  0x2EC0950



// Camera matrix getters

#define OFFSET_CAMERA_GET_WORLDTOCAMERAMATRIX      0x2EC16F0

#define OFFSET_CAMERA_GET_PROJECTIONMATRIX         0x2EC13A0



// WeaponryController offsets

// PlayerController.<bzwy>k__BackingField (WeaponryController) at 0xD0

// WeaponryController.<caca>k__BackingField (current WeaponController) at 0xA0

#define OFFSET_WEAPONRYCONTROLLER                  0xD0

#define OFFSET_WEAPONCONTROLLER                    0xA0



// GunController fields (short ammo at 0xE4 / 0xE6)

#define OFFSET_CURRENT_AMMO                        0xE4  // short cegx

#define OFFSET_MAX_AMMO                            0xE6  // short cegy



// GunController methods (wyw() / wyy())

#define OFFSET_GUNCONTROLLER_GETCURRENTAMMO        0x9990E0  // get_ddtj -> cegx

#define OFFSET_GUNCONTROLLER_GETMAXAMMO            0x999160  // get_ddtk -> cegy



uintptr_t base = 0;

uintptr_t unityPlayerBase = 0;



// Matrix для trail

Matrix16 g_ViewProjectionMatrix = { 0 };

bool g_MatrixValid = false;



// Bind system

struct BindConfig {

    int key;

    bool toggleMode;

};



BindConfig psBind = { 0, false };

BindConfig jbBind = { 0, false };

BindConfig airJumpBind = { 0, false };

BindConfig velocityBind = { 0, false };

BindConfig trailBind = { 0, false };

BindConfig ammoBind = { 0, false };

BindConfig espBind = { 0, false };



int* waitingForBind = nullptr;



std::vector<Vector3> trailPoints;



// Feature states

bool pixelSurf = false;

bool jbActive = false;

bool airJump = false;

bool showVelocity = false;

bool showTrail = false;

bool infinityAmmo = false;



float lastSpeed = 0.0f;

bool wasSurfing = false;

float surfSpeed = 1.5f;



// ESP система

bool boxEsp = false;

bool b_HookEnemyCords = false;

bool b_HookLocalCords = false;

bool b_LocalCordshooked = false;

bool b_EnemyCordshooked = false;



BYTE* LocalCords = nullptr;

BYTE* EnemyCoordsBuffer = nullptr;

BYTE* OldLocalCordsFunc = nullptr;

BYTE* NewLocalCordsFunc = nullptr;

BYTE* OldEnemyCordsFunc = nullptr;

BYTE* NewEnemyCordsFunc = nullptr;



int espCount = 10;

// IL2CPP runtime bindings (init in HackThread)
IL2CPP_API g_il2cpp;
#include <cstdio>
static void LINDY_LOG(const char* fmt, ...) {
    FILE* f = nullptr; fopen_s(&f, "C:\\Temp\\lindy_esp.log", "a");
    if (!f) return;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

// Forward-declare the transform function ptr (defined later near IL2CPP globals block).
extern Vector3(__fastcall* o_Transform_get_position)(uintptr_t);

Il2CppClass* g_PlayerManagerClass = nullptr;
Il2CppField* g_PlayerManagerInstanceField = nullptr;

static void* GetPlayerManagerInstance() {
    if (!g_il2cpp.field_static_get_value || !g_PlayerManagerInstanceField) return nullptr;
    void* inst = nullptr;
    g_il2cpp.field_static_get_value(g_PlayerManagerInstanceField, &inst);
    return inst;
}

// Read player Dictionary<int, PlayerController*> and dump values to out.
// Il2Cpp Dictionary layout (Mono/Il2Cpp ~= .NET reference):
//   0x00 vtable/klass ptr
//   0x08 monitor
//   0x10 buckets (int[]) 
//   0x18 entries (Entry[])
//   Entry: 0x00 hashCode(int32) 0x04 next(int32) 0x08 key 0x10 value ... size = 0x18 for <int,ref>
//   0x20 count(int32)
// This is empirical and may vary; if it crashes, fall back to key-string dict at 0x50 or use ToArray via method invocation.
static void CollectPlayers(void** out, int maxN, int& outN) {
    outN = 0;
    void* pm = GetPlayerManagerInstance();
    if (!pm) return;
    uintptr_t dict = *(uintptr_t*)((uintptr_t)pm + PM_PLAYERS_DICT);
    if (!dict) return;
    __try {
        uintptr_t entries = *(uintptr_t*)(dict + 0x18);
        int count = *(int*)(dict + 0x20);
        if (!entries || count <= 0 || count > 256) return;
        // il2cpp array header: 0x10 -> length, 0x20 -> first element (approximate)
        uintptr_t arr = entries + 0x20;
        const int ENTRY_SIZE = 0x18;
        for (int i = 0; i < count && outN < maxN; ++i) {
            uintptr_t e = arr + (uintptr_t)i * ENTRY_SIZE;
            int hash = *(int*)(e + 0x00);
            if (hash < 0) continue;
            void* pc = *(void**)(e + 0x10);
            if (pc) out[outN++] = pc;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outN = 0;
    }
}

static bool GetPCPosition(void* pc, Vector3& outPos) {
    if (!pc || !o_Transform_get_position) return false;
    __try {
        uintptr_t tf = *(uintptr_t*)((uintptr_t)pc + PC_MAIN_CAMERA_HOLDER);
        if (!tf) return false;
        outPos = o_Transform_get_position(tf);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool IsBotPC(void* pc) {
    if (!pc) return false;
    __try {
        return *(void**)((uintptr_t)pc + PC_BOT_CONTROLLER) != nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void* GetLocalPC() {
    void* pm = GetPlayerManagerInstance();
    if (!pm) return nullptr;
    __try {
        return *(void**)((uintptr_t)pm + PM_LOCAL_PLAYER_BF);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

float espMaxDistance = 100.0f;



// ESP code arrays (из тестового файла)

struct Vector3d {

    double x, y, z;

    Vector3 ToFloat() const {

        return Vector3((float)x, (float)y, (float)z);

    }

};

int maxTrailPoints = 600;

float trailMinDistance = 5.0f; // Уменьшил для более плавного trail



// Tabs

int currentTab = 0;



// Config menu

bool showConfigMenu = false;

float menuColor[3] = { 0.1f, 0.1f, 0.15f }; // RGB для цвета меню

float accentColor[3] = { 0.4f, 0.6f, 1.0f }; // RGB для акцентного цвета

bool useCustomBackground = false;

ID3D11ShaderResourceView* backgroundTexture = nullptr;

bool backgroundLoaded = false;

char backgroundPath[256] = "maxresdefault.jpg"; // Default path



// Load background texture from custom path

bool LoadBackgroundTexture() {

    if (backgroundTexture) {

        backgroundTexture->Release();

        backgroundTexture = nullptr;

    }



    // Convert to wide string for WIC

    wchar_t widePath[512];

    MultiByteToWideChar(CP_ACP, 0, backgroundPath, -1, widePath, 512);



    // Initialize COM

    CoInitialize(nullptr);



    IWICImagingFactory* factory = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&factory);



    if (FAILED(hr)) {

        backgroundLoaded = false;

        return false;

    }



    IWICBitmapDecoder* decoder = nullptr;

    hr = factory->CreateDecoderFromFilename(widePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);



    if (FAILED(hr)) {

        factory->Release();

        backgroundLoaded = false;

        return false;

    }



    IWICBitmapFrameDecode* frame = nullptr;

    hr = decoder->GetFrame(0, &frame);



    if (FAILED(hr)) {

        decoder->Release();

        factory->Release();

        backgroundLoaded = false;

        return false;

    }



    UINT width, height;

    frame->GetSize(&width, &height);



    // Create texture (simplified - just mark as loaded for now)

    frame->Release();

    decoder->Release();

    factory->Release();



    backgroundLoaded = true;

    return true;

}



bool(__fastcall* o_CC_get_isGrounded)(uintptr_t) = nullptr;

int(__fastcall* o_CC_Move)(uintptr_t, Vector3) = nullptr;

Vector3(__fastcall* o_CC_get_velocity)(uintptr_t) = nullptr;

Vector3(__fastcall* o_Transform_get_position)(uintptr_t) = nullptr;

uintptr_t(__fastcall* o_GetPlayerController)() = nullptr;

uintptr_t(__fastcall* o_Camera_get_main)() = nullptr;

Vector3(__fastcall* o_WorldToScreenPoint)(uintptr_t, Vector3) = nullptr;

Matrix16(__fastcall* o_Camera_get_worldToCameraMatrix)(uintptr_t) = nullptr;

Matrix16(__fastcall* o_Camera_get_projectionMatrix)(uintptr_t) = nullptr;

short(__fastcall* o_GunController_GetCurrentAmmo)(uintptr_t) = nullptr;



uintptr_t GetCamera() {

    if (!base) return 0;

    __try { return o_Camera_get_main(); }

    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

}



uintptr_t GetPlayerController() {

    if (!base) return 0;

    __try { return o_GetPlayerController(); }

    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

}



// Hook на GunController::gcloafhgkcb() - возвращает текущие патроны

short __fastcall hk_GunController_GetCurrentAmmo(uintptr_t instance)

{

    short currentAmmo = o_GunController_GetCurrentAmmo(instance);



    if (keyValidated && infinityAmmo && instance) {

        __try {

            // Читаем максимальные патроны из GunController

            short maxAmmo = *(short*)(instance + OFFSET_MAX_AMMO);



            // Всегда возвращаем максимум если патроны есть

            if (maxAmmo > 0 && maxAmmo < 500) {

                *(short*)(instance + OFFSET_CURRENT_AMMO) = maxAmmo;

                return maxAmmo;

            }

        }

        __except (EXCEPTION_EXECUTE_HANDLER) {}

    }



    return currentAmmo;

}



void InfinityAmmoLoop() {

    if (!keyValidated || !base) return;



    __try {

        uintptr_t localPlayer = GetPlayerController();

        if (!localPlayer) return;



        uintptr_t weaponryController = *(uintptr_t*)(localPlayer + OFFSET_WEAPONRYCONTROLLER);

        if (!weaponryController) return;



        uintptr_t weaponController = *(uintptr_t*)(weaponryController + OFFSET_WEAPONCONTROLLER);

        if (!weaponController) return;



        // Проверяем что это GunController (не нож)

        short maxAmmo = *(short*)(weaponController + OFFSET_MAX_AMMO);

        if (maxAmmo > 0 && maxAmmo < 1000) {

            // Infinity Ammo

            if (infinityAmmo) {

                *(short*)(weaponController + OFFSET_CURRENT_AMMO) = maxAmmo;

            }

        }

    }

    __except (EXCEPTION_EXECUTE_HANDLER) {}

}



uintptr_t GetTransform() {

    uintptr_t pc = GetPlayerController();

    if (!pc) return 0;

    __try { return *(uintptr_t*)(pc + 0x10); }

    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

}



Vector3 GetPlayerPosition() {

    uintptr_t transform = GetTransform();

    if (transform) { return o_Transform_get_position(transform); }

    return Vector3(0, 0, 0);

}



// Чтение указателя с оффсетами

uintptr_t ReadPtr(uintptr_t base, const std::vector<unsigned int>& offsets) {

    uintptr_t addr = base;

    __try {

        for (unsigned int offset : offsets) {

            if (addr < 0x10000) return 0;

            addr = *(uintptr_t*)addr;

            addr += offset;

        }

    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        return 0;

    }

    return addr;

}



// WorldToScreen через матрицу (как в тестовом файле)

bool GetViewProjectionMatrix(Matrix16& out) {

    if (!o_Camera_get_worldToCameraMatrix || !o_Camera_get_projectionMatrix) return false;

    uintptr_t cam = GetCamera();

    if (!cam) return false;

    __try {

        Matrix16 view = o_Camera_get_worldToCameraMatrix(cam);

        Matrix16 proj = o_Camera_get_projectionMatrix(cam);

        // VP = Projection * View (row-major)

        for (int r = 0; r < 4; r++) {

            for (int c = 0; c < 4; c++) {

                float sum = 0.f;

                for (int k = 0; k < 4; k++) {

                    sum += ((float*)&view)[r * 4 + k] * ((float*)&proj)[k * 4 + c];

                }

                ((float*)&out)[r * 4 + c] = sum;

            }

        }

        return true;

    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        return false;

    }

}



bool WorldToScreen(const Vector3& pos, Vector2& screen, const Matrix16& matrix) {

    Vector4 clip;

    clip.x = pos.x * matrix.a + pos.y * matrix.e + pos.z * matrix.i + matrix.m;

    clip.y = pos.x * matrix.b + pos.y * matrix.f + pos.z * matrix.j + matrix.n;

    clip.z = pos.x * matrix.c + pos.y * matrix.g + pos.z * matrix.k + matrix.o;

    clip.w = pos.x * matrix.d + pos.y * matrix.h + pos.z * matrix.l + matrix.p;



    const float epsilon = 1e-6f;

    if (clip.w <= epsilon)

        return false; // точка позади камеры



    float invW = 1.0f / clip.w;

    float ndcX = clip.x * invW;

    float ndcY = clip.y * invW;



    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    

    // Преобразование в экранные координаты (с переворотом Y)

    screen.x = (ndcX + 1.0f) * 0.5f * displaySize.x;

    screen.y = (1.0f - ndcY) * 0.5f * displaySize.y;



    return true;

}



bool __fastcall hk_CC_get_isGrounded(uintptr_t instance)

{

    if (!keyValidated) return o_CC_get_isGrounded(instance);

    if (airJump && instance) return true;

    if (jbActive) return false;

    return o_CC_get_isGrounded(instance);

}



Vector3 __fastcall hk_CC_get_velocity(uintptr_t instance)

{

    Vector3 vel = o_CC_get_velocity(instance);

    if (keyValidated) {

        lastSpeed = sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);

        if (jbActive) {

            float horSpeed = sqrt(vel.x * vel.x + vel.z * vel.z);

            if (horSpeed < 1.0f) horSpeed = surfSpeed * 5.0f;

            if (vel.Length2D() < surfSpeed * 3.0f) {

                vel.x *= 1.05f;

                vel.z *= 1.05f;

            }

            vel.y = 0;

        }

    }

    return vel;

}



int __fastcall hk_CC_Move(uintptr_t instance, Vector3 motion)

{

    if (keyValidated && jbActive) {

        float speed = sqrt(motion.x * motion.x + motion.z * motion.z);

        if (speed > 0.1f) {

            motion.x = (motion.x / speed) * surfSpeed * 10.0f;

            motion.z = (motion.z / speed) * surfSpeed * 10.0f;

        }

        motion.y = 0;

    }

    return o_CC_Move(instance, motion);

}



void UpdateTrail() {

    if (!keyValidated) { 

        trailPoints.clear(); 

        return; 

    }

    

    if (!showTrail) {

        // Не очищаем сразу, чтобы trail плавно исчезал

        if (!trailPoints.empty()) {

            trailPoints.clear();

        }

        wasSurfing = false;

        return;

    }

    

    Vector3 pos = GetPlayerPosition();

    

    // Проверяем что позиция валидная

    if (pos.x == 0 && pos.y == 0 && pos.z == 0) {

        return;

    }

    

    // Добавляем точки независимо от серфа

    if (trailPoints.empty() || pos.Distance(trailPoints.back()) > trailMinDistance) {

        trailPoints.push_back(pos);

        if (trailPoints.size() > maxTrailPoints) {

            trailPoints.erase(trailPoints.begin());

        }

    }

}




void BoxEsp() {
    static int __log_ctr = 0; ++__log_ctr;
    bool __do_log = (__log_ctr % 120) == 1;
    if (!g_il2cpp.ga || !g_PlayerManagerInstanceField) { if (__do_log) LINDY_LOG("[BoxEsp] blocked ga=%p field=%p", (void*)g_il2cpp.ga, (void*)g_PlayerManagerInstanceField); return; }
    if (__do_log) {
        void* __pm = GetPlayerManagerInstance();
        void* __localPC = GetLocalPC();
        Vector3 __mp(0,0,0); if (__localPC) GetPCPosition(__localPC, __mp);
        LINDY_LOG("[BoxEsp] probe pm=%p localPC=%p myPos=(%.2f,%.2f,%.2f)", __pm, __localPC, __mp.x, __mp.y, __mp.z);
    }

    void* localPC = GetLocalPC();
    Vector3 MyPos(0,0,0);
    if (localPC) {
        GetPCPosition(localPC, MyPos);
    } else {
        MyPos = GetPlayerPosition();
    }

    Matrix16 vpMatrix;
    if (!GetViewProjectionMatrix(vpMatrix)) return;
    g_ViewProjectionMatrix = vpMatrix;
    g_MatrixValid = true;

    void* players[64];
    int nPlayers = 0;
    CollectPlayers(players, 64, nPlayers);
    if (nPlayers <= 0) return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    int drawn = 0;
    for (int i = 0; i < nPlayers && drawn < espCount; ++i) {
        void* pc = players[i];
        if (!pc || pc == localPC) continue;
        Vector3 EnemyPos;
        if (!GetPCPosition(pc, EnemyPos)) continue;
        Vector2 ScreenPos;
        if (!WorldToScreen(EnemyPos, ScreenPos, vpMatrix)) continue;
        float distance = (EnemyPos - MyPos).Length();
        if (distance > espMaxDistance || distance < 0.5f) continue;
        float scale = (10.0f / distance);
        float dynamicThickness = fmaxf(1.0f, fminf(3.0f, (2.5f * scale * 2)));
        float fW = 75.f * scale;
        float fH = 150.f * scale;
        ImVec2 box_min = ImVec2(ScreenPos.x - fW * 0.5f, ScreenPos.y - fH * 0.5f);
        ImVec2 box_max = ImVec2(ScreenPos.x + fW * 0.5f, ScreenPos.y + fH * 0.5f);
        ImU32 color = IsBotPC(pc) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
        draw_list->AddRect(box_min, box_max, color, 2.5f, 0, dynamicThickness);
        char distText[32];
        snprintf(distText, sizeof(distText), "%.0fm", distance);
        ImVec2 textSize = ImGui::CalcTextSize(distText);
        ImVec2 textPos = ImVec2(ScreenPos.x - textSize.x * 0.5f, box_max.y + 2);
        draw_list->AddText(ImVec2(textPos.x - 1, textPos.y), IM_COL32(0, 0, 0, 255), distText);
        draw_list->AddText(ImVec2(textPos.x + 1, textPos.y), IM_COL32(0, 0, 0, 255), distText);
        draw_list->AddText(ImVec2(textPos.x, textPos.y - 1), IM_COL32(0, 0, 0, 255), distText);
        draw_list->AddText(ImVec2(textPos.x, textPos.y + 1), IM_COL32(0, 0, 0, 255), distText);
        draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), distText);
        ++drawn;
    }
}


void DrawTrail() {

    if (!keyValidated || !showTrail || trailPoints.size() < 2) return;

    if (!unityPlayerBase) return;



    // Обновляем матрицу через камерные методы

    if (!GetViewProjectionMatrix(g_ViewProjectionMatrix)) return;

    g_MatrixValid = true;

    

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    

    for (size_t i = 0; i < trailPoints.size() - 1; i++) {

        Vector2 screen1, screen2;

        

        if (WorldToScreen(trailPoints[i], screen1, g_ViewProjectionMatrix) && 

            WorldToScreen(trailPoints[i + 1], screen2, g_ViewProjectionMatrix)) {

            // Градиент от старых точек к новым

            float alpha = (float)(i + 1) / (float)trailPoints.size();

            alpha = alpha * alpha; // Квадратичная интерполяция для плавности

            

            // Цвет от синего к белому с градиентом прозрачности

            ImU32 color = IM_COL32(

                (int)(100 + 155 * alpha),  // R: 100 -> 255

                (int)(150 + 105 * alpha),  // G: 150 -> 255

                255,                        // B: 255

                (int)(100 + 155 * alpha)   // A: 100 -> 255

            );

            

            // Толщина линии тоже меняется

            float thickness = 1.0f + 2.0f * alpha;

            

            drawList->AddLine(

                ImVec2(screen1.x, screen1.y), 

                ImVec2(screen2.x, screen2.y), 

                color, 

                thickness

            );

        }

    }

    

    // Добавляем точку на текущей позиции игрока

    if (!trailPoints.empty()) {

        Vector2 currentScreen;

        if (WorldToScreen(trailPoints.back(), currentScreen, g_ViewProjectionMatrix)) {

            drawList->AddCircleFilled(

                ImVec2(currentScreen.x, currentScreen.y), 

                4.0f, 

                IM_COL32(255, 255, 255, 255)

            );

        }

    }

}



std::string GetKeyName(int key) {

    if (key == 0) return "None";

    if (key == VK_XBUTTON1) return "Mouse4";

    if (key == VK_XBUTTON2) return "Mouse5";

    if (key == VK_LBUTTON) return "LMB";

    if (key == VK_RBUTTON) return "RMB";

    if (key == VK_MBUTTON) return "MMB";

    if (key >= VK_F1 && key <= VK_F12) return "F" + std::to_string(key - VK_F1 + 1);

    if (key >= 'A' && key <= 'Z') return std::string(1, (char)key);

    if (key >= '0' && key <= '9') return std::string(1, (char)key);

    if (key == VK_SPACE) return "Space";

    if (key == VK_SHIFT) return "Shift";

    if (key == VK_CONTROL) return "Ctrl";

    if (key == VK_MENU) return "Alt";

    if (key == VK_TAB) return "Tab";

    if (key == VK_CAPITAL) return "Caps";

    return "Key" + std::to_string(key);

}



DWORD WINAPI KeyListenerThread(LPVOID) {

    while (!unloadRequested) {

        if (keyValidated) {

            if (psBind.key > 0) {

                bool held = (GetAsyncKeyState(psBind.key) & 0x8000) != 0;

                static bool psToggle = false, psLast = false;

                if (psBind.toggleMode) { if (held && !psLast) psToggle = !psToggle; pixelSurf = psToggle; }

                else pixelSurf = held;

                psLast = held;

            }

            if (jbBind.key > 0) {

                bool held = (GetAsyncKeyState(jbBind.key) & 0x8000) != 0;

                static bool jbToggle = false, jbLast = false;

                if (jbBind.toggleMode) { if (held && !jbLast) jbToggle = !jbToggle; jbActive = jbToggle; }

                else jbActive = held;

                jbLast = held;

            }

            if (airJumpBind.key > 0) {

                bool held = (GetAsyncKeyState(airJumpBind.key) & 0x8000) != 0;

                static bool ajToggle = false, ajLast = false;

                if (airJumpBind.toggleMode) { if (held && !ajLast) ajToggle = !ajToggle; airJump = ajToggle; }

                else airJump = held;

                ajLast = held;

            }

            if (velocityBind.key > 0) {

                bool held = (GetAsyncKeyState(velocityBind.key) & 0x8000) != 0;

                static bool velToggle = false, velLast = false;

                if (velocityBind.toggleMode) { if (held && !velLast) velToggle = !velToggle; showVelocity = velToggle; }

                else showVelocity = held;

                velLast = held;

            }

            if (trailBind.key > 0) {

                bool held = (GetAsyncKeyState(trailBind.key) & 0x8000) != 0;

                static bool trailToggle = false, trailLast = false;

                if (trailBind.toggleMode) { if (held && !trailLast) trailToggle = !trailToggle; showTrail = trailToggle; }

                else showTrail = held;

                trailLast = held;

            }

            if (ammoBind.key > 0) {

                bool held = (GetAsyncKeyState(ammoBind.key) & 0x8000) != 0;

                static bool ammoToggle = false, ammoLast = false;

                if (ammoBind.toggleMode) { if (held && !ammoLast) ammoToggle = !ammoToggle; infinityAmmo = ammoToggle; }

                else infinityAmmo = held;

                ammoLast = held;

            }

            if (espBind.key > 0) {

                bool held = (GetAsyncKeyState(espBind.key) & 0x8000) != 0;

                static bool espToggle = false, espLast = false;

                if (espBind.toggleMode) { 

                    if (held && !espLast) {

                        espToggle = !espToggle;

                        boxEsp = espToggle;

                        b_HookEnemyCords = espToggle;

                        b_HookLocalCords = espToggle;

                    }

                }

                else {

                    boxEsp = held;

                    b_HookEnemyCords = held;

                    b_HookLocalCords = held;

                }

                espLast = held;

            }

        }

        Sleep(10);

    }

    return 0;

}



DWORD WINAPI TrailThread(LPVOID) {

    
    
    

    while (!unloadRequested) {

        UpdateTrail();

        InfinityAmmoLoop();
        Sleep(16);

    }

    return 0;

}



void InitImGui()

{

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    ImGui_ImplWin32_Init(window);

    ImGui_ImplDX11_Init(pDevice, pContext);

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 28.0f);

}



LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (keyValidated && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;

    if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;



    if (keyValidated && waitingForBind && (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN || uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)) {

        int key = 0;

        if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) key = (int)wParam;

        else if (uMsg == WM_LBUTTONDOWN) key = VK_LBUTTON;

        else if (uMsg == WM_RBUTTONDOWN) key = VK_RBUTTON;

        else if (uMsg == WM_MBUTTONDOWN) key = VK_MBUTTON;

        else if (uMsg == WM_XBUTTONDOWN) key = (HIWORD(wParam) == 1) ? VK_XBUTTON1 : VK_XBUTTON2;



        if (key == VK_ESCAPE) {

            waitingForBind = nullptr;

            return true;

        }



        *waitingForBind = key;

        waitingForBind = nullptr;

        return true;

    }



    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);

}



HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)

{

    if (unloadRequested) {

        // Просто отключаем меню и хуки, но не крашим игру

        keyValidated = false;

        return oPresent(pSwapChain, SyncInterval, Flags);

    }



    if (!init)

    {

        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))

        {

            pDevice->GetImmediateContext(&pContext);

            DXGI_SWAP_CHAIN_DESC sd;

            pSwapChain->GetDesc(&sd);

            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer = NULL;

            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

            if (pBackBuffer) {

                pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);

                pBackBuffer->Release();

            }

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            InitImGui();

            init = true;

        }

        else return oPresent(pSwapChain, SyncInterval, Flags);

    }



    ImGui_ImplDX11_NewFrame();

    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();



    if (!keyValidated) {

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Enter Key", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("t.me/ze0ntap");

        static char key[64] = "";

        ImGui::InputText("##key", key, 64);

        if (ImGui::Button("Submit") || (ImGui::IsKeyPressed(ImGuiKey_Enter) && strlen(key) > 0)) {

            if (strcmp(key, "ze0ntap") == 0) keyValidated = true;

        }

        ImGui::End();

        ImGui::Render();

        pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return oPresent(pSwapChain, SyncInterval, Flags);

    }



    DrawTrail();

    

    // Render ESP if enabled

    if (boxEsp) {

        BoxEsp();

    }



    // Config window

    if (showConfigMenu) {

        ImGui::Begin("Config", &showConfigMenu);

        ImGui::Text("Menu Customization");

        ImGui::Separator();

        ImGui::ColorEdit3("Menu Color", menuColor);

        ImGui::ColorEdit3("Accent Color", accentColor);



        ImGui::Checkbox("Custom Background", &useCustomBackground);

        if (ImGui::IsItemHovered()) {

            ImGui::SetTooltip("Enable custom background image");

        }



        if (useCustomBackground) {

            ImGui::Text("Background Path:");

            ImGui::InputText("##bgpath", backgroundPath, sizeof(backgroundPath));

            if (ImGui::IsItemHovered()) {

                ImGui::SetTooltip("Enter path to image file (jpg, png, etc.)");

            }



            if (ImGui::Button("Load Background")) {

                backgroundLoaded = false; // Reset to reload

                if (LoadBackgroundTexture()) {

                    // Success

                }

                else {

                    // File not found - could show error message

                }

            }

            ImGui::SameLine();



            if (backgroundLoaded) {

                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Loaded!");

            }

            else {

                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not found");

            }

        }



        ImGui::End();

    }



    // Modern dark theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(18.0f, 16.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(10.0f, 9.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.WindowRounding = 12.0f;
    style.ChildRounding = 9.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    const ImVec4 accent(accentColor[0], accentColor[1], accentColor[2], 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.94f, 0.97f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.46f, 0.50f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.035f, 0.043f, 0.060f, 0.98f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.055f, 0.065f, 0.088f, 0.96f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.045f, 0.054f, 0.074f, 0.99f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.15f, 0.18f, 0.24f, 0.85f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.085f, 0.10f, 0.135f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(accent.x, accent.y, accent.z, 0.16f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(accent.x, accent.y, accent.z, 0.24f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.085f, 0.10f, 0.135f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(accent.x, accent.y, accent.z, 0.22f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(accent.x, accent.y, accent.z, 0.34f);
    style.Colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.16f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.24f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.34f);
    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(accent.x, accent.y, accent.z, 0.72f);
    style.Colors[ImGuiCol_SliderGrabActive] = accent;
    style.Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.17f, 0.23f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(accent.x, accent.y, accent.z, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent.x, accent.y, accent.z, 0.55f);
    style.Colors[ImGuiCol_ResizeGripActive] = accent;

    ImGui::SetNextWindowSize(ImVec2(720.0f, 520.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("ze0nware", nullptr, ImGuiWindowFlags_NoCollapse);

    // Custom background

    if (useCustomBackground && backgroundTexture) {

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 window_pos = ImGui::GetWindowPos();

        ImVec2 window_size = ImGui::GetWindowSize();



        // Draw background with some transparency

        draw_list->AddImageQuad(

            backgroundTexture,

            window_pos,

            ImVec2(window_pos.x + window_size.x, window_pos.y),

            ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y),

            ImVec2(window_pos.x, window_pos.y + window_size.y),

            ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1),

            IM_COL32(255, 255, 255, 100) // Semi-transparent

        );

    }



    // Header
    ImGui::TextColored(accent, "ZE0NWARE");
    ImGui::SameLine();
    ImGui::TextDisabled("/ control center");
    ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);
    if (ImGui::Button("SETTINGS", ImVec2(84.0f, 30.0f)))
        showConfigMenu = !showConfigMenu;

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    const float tabWidth = (ImGui::GetContentRegionAvail().x - 20.0f) / 3.0f;
    const char* tabNames[] = { "MOVEMENT", "VISUALS", "WEAPONS" };
    for (int tab = 0; tab < 3; ++tab) {
        if (tab > 0) ImGui::SameLine();
        if (currentTab == tab) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(accent.x, accent.y, accent.z, 0.24f));
            ImGui::PushStyleColor(ImGuiCol_Text, accent);
        }
        if (ImGui::Button(tabNames[tab], ImVec2(tabWidth, 36.0f)))
            currentTab = tab;
        if (currentTab == tab)
            ImGui::PopStyleColor(2);
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::BeginChild("##feature_panel", ImVec2(0.0f, -72.0f), true);
    ImGui::TextColored(accent, currentTab == 0 ? "MOVEMENT FEATURES" : (currentTab == 1 ? "VISUAL FEATURES" : "WEAPON FEATURES"));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 3.0f));

    if (currentTab == 0) {

        // Movement tab

        ImGui::Checkbox("PS", &pixelSurf);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &psBind.key ? "[...]##ps" : (psBind.key > 0 ? GetKeyName(psBind.key).c_str() : "None"))) waitingForBind = &psBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##ps")) { psBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##ps", &psBind.toggleMode);



        ImGui::Checkbox("JB", &jbActive);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &jbBind.key ? "[...]##jb" : (jbBind.key > 0 ? GetKeyName(jbBind.key).c_str() : "None"))) waitingForBind = &jbBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##jb")) { jbBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##jb", &jbBind.toggleMode);



        if (jbActive) ImGui::SliderFloat("Surf Speed", &surfSpeed, 0.5f, 3.0f);



        ImGui::Checkbox("AirJump", &airJump);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &airJumpBind.key ? "[...]##aj" : (airJumpBind.key > 0 ? GetKeyName(airJumpBind.key).c_str() : "None"))) waitingForBind = &airJumpBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##aj")) { airJumpBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##aj", &airJumpBind.toggleMode);

    }

    else if (currentTab == 1) {

        // Visuals tab

        ImGui::Checkbox("Show Velocity", &showVelocity);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &velocityBind.key ? "[...]##vel" : (velocityBind.key > 0 ? GetKeyName(velocityBind.key).c_str() : "None"))) waitingForBind = &velocityBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##vel")) { velocityBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##vel", &velocityBind.toggleMode);



        ImGui::Checkbox("Show Trail", &showTrail);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &trailBind.key ? "[...]##trail" : (trailBind.key > 0 ? GetKeyName(trailBind.key).c_str() : "None"))) waitingForBind = &trailBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##trail")) { trailBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##trail", &trailBind.toggleMode);

        

        if (showTrail) {

            ImGui::SliderInt("Trail Length", &maxTrailPoints, 100, 1000);

            ImGui::SliderFloat("Trail Distance", &trailMinDistance, 1.0f, 20.0f);

            ImGui::Text("Trail Points: %d", (int)trailPoints.size());

        }

        

        ImGui::Separator();

        

        ImGui::Checkbox("Box ESP", &boxEsp);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &espBind.key ? "[...]##esp" : (espBind.key > 0 ? GetKeyName(espBind.key).c_str() : "None"))) waitingForBind = &espBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##esp")) { espBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##esp", &espBind.toggleMode);

        

        if (boxEsp) {

            // Автоматически включаем хуки

                        ImGui::SliderInt("ESP Count", &espCount, 1, 20);

            ImGui::SliderFloat("ESP Distance", &espMaxDistance, 10.0f, 200.0f);

            ImGui::Text("ESP mode: IL2CPP (PlayerManager)");

        } else {

            // Отключаем хуки когда ESP выключен

                    }

    }

    else if (currentTab == 2) {

        // Weapons tab

        ImGui::Checkbox("Infinity Ammo", &infinityAmmo);

        ImGui::SameLine();

        if (ImGui::Button(waitingForBind == &ammoBind.key ? "[...]##ammo" : (ammoBind.key > 0 ? GetKeyName(ammoBind.key).c_str() : "None"))) waitingForBind = &ammoBind.key;

        ImGui::SameLine();

        if (ImGui::Button("X##ammo")) { ammoBind.key = 0; }

        ImGui::SameLine();

        ImGui::Checkbox("Toggle##ammo", &ammoBind.toggleMode);

    }



    ImGui::EndChild();

    ImGui::TextDisabled("INSERT  menu  |  END  unload");
    ImGui::SameLine(ImGui::GetWindowWidth() - 122.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.58f, 0.12f, 0.16f, 0.72f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.16f, 0.20f, 0.90f));
    if (ImGui::Button("UNLOAD", ImVec2(86.0f, 30.0f))) {
        unloadRequested = true;
        keyValidated = false;
    }
    ImGui::PopStyleColor(2);

    ImGui::End();




    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    if (pixelSurf) {

        ImVec2 ts = ImGui::CalcTextSize("ps");

        float x = (ImGui::GetIO().DisplaySize.x - ts.x) * 0.5f, y = ImGui::GetIO().DisplaySize.y - 100.0f;

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x + 2, y + 2), IM_COL32(0, 0, 0, 160), "ps");

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), IM_COL32(100, 255, 140, 255), "ps");

    }

    if (jbActive) {

        ImVec2 ts = ImGui::CalcTextSize("jb");

        float x = (ImGui::GetIO().DisplaySize.x - ts.x) * 0.5f, y = ImGui::GetIO().DisplaySize.y - 140.0f;

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x + 2, y + 2), IM_COL32(0, 0, 0, 160), "jb");

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), IM_COL32(100, 255, 140, 255), "jb");

    }

    if (showVelocity && lastSpeed > 0.5f) {

        char t[32]; sprintf_s(t, "%d", (int)lastSpeed);

        ImVec2 ts = ImGui::CalcTextSize(t);

        float x = (ImGui::GetIO().DisplaySize.x - ts.x) * 0.5f, y = ImGui::GetIO().DisplaySize.y - 190.0f;

        ImU32 c = lastSpeed >= 250 ? IM_COL32(80, 220, 80, 255) : (lastSpeed >= 150 ? IM_COL32(255, 165, 40, 255) : IM_COL32(220, 60, 60, 255));

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x + 2, y + 2), IM_COL32(0, 0, 0, 160), t);

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), c, t);

    }

    ImGui::PopFont();

    ImGui::Render();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);

}



DWORD WINAPI HackThread(LPVOID)

{

    MH_Initialize();

    // IL2CPP init: locate PlayerManager and its LazySingleton<T>.cgxr static field
        bool __ok_init = g_il2cpp.init();
    LINDY_LOG("[init] g_il2cpp.init=%d ga=%p", (int)__ok_init, (void*)g_il2cpp.ga);
    if (__ok_init) {
        g_PlayerManagerClass = g_il2cpp.find_class("", "PlayerManager");
        LINDY_LOG("[init] PlayerManagerClass=%p", (void*)g_PlayerManagerClass);
        if (g_PlayerManagerClass) {
            // The static instance field lives on the LazySingleton<PlayerManager> parent as 'cgxr'.
            Il2CppClass* parent = g_il2cpp.class_get_parent ? g_il2cpp.class_get_parent(g_PlayerManagerClass) : nullptr;
            if (parent) {
                g_PlayerManagerInstanceField = g_il2cpp.class_get_field_from_name(parent, "cgxr");
            }
            if (!g_PlayerManagerInstanceField) {
                // fallback: try on class itself
                g_PlayerManagerInstanceField = g_il2cpp.class_get_field_from_name(g_PlayerManagerClass, "cgxr");
            }
        }
    }
    base = (uintptr_t)GetModuleHandleA("GameAssembly.dll");

    unityPlayerBase = (uintptr_t)GetModuleHandleA("UnityPlayer.dll");

    if (!base || !unityPlayerBase) return 0;

    

    g_MatrixValid = true;



    o_GetPlayerController = (uintptr_t(__fastcall*)())(base + OFFSET_GET_PLAYERCONTROLLER);

    o_Transform_get_position = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_POSITION);

    o_Camera_get_main = (uintptr_t(__fastcall*)())(base + OFFSET_CAMERA_MAIN);

    o_WorldToScreenPoint = (Vector3(__fastcall*)(uintptr_t, Vector3))(base + OFFSET_WORLDTOSCREENPOINT);

    o_Camera_get_worldToCameraMatrix = (Matrix16(__fastcall*)(uintptr_t))(base + OFFSET_CAMERA_GET_WORLDTOCAMERAMATRIX);

    o_Camera_get_projectionMatrix = (Matrix16(__fastcall*)(uintptr_t))(base + OFFSET_CAMERA_GET_PROJECTIONMATRIX);



    // CharacterController hooks

    MH_CreateHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_GET_ISGROUNDED), hk_CC_get_isGrounded, (LPVOID*)&o_CC_get_isGrounded);

    MH_EnableHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_GET_ISGROUNDED));

    MH_CreateHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_MOVE), hk_CC_Move, (LPVOID*)&o_CC_Move);

    MH_EnableHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_MOVE));

    MH_CreateHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_GET_VELOCITY), hk_CC_get_velocity, (LPVOID*)&o_CC_get_velocity);

    MH_EnableHook((LPVOID)(base + OFFSET_CHARACTERCONTROLLER_GET_VELOCITY));



    // GunController hook for infinity ammo

    MH_CreateHook((LPVOID)(base + OFFSET_GUNCONTROLLER_GETCURRENTAMMO), hk_GunController_GetCurrentAmmo, (LPVOID*)&o_GunController_GetCurrentAmmo);

    MH_EnableHook((LPVOID)(base + OFFSET_GUNCONTROLLER_GETCURRENTAMMO));



    return 0;

}



DWORD WINAPI MainThread(LPVOID lpReserved)

{

    bool init_hook = false;

    do {

        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {

            kiero::bind(8, (void**)&oPresent, hkPresent);

            init_hook = true;

        }

    } while (!init_hook);

    return TRUE;

}



BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)

{

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        LINDY_LOG("[DllMain] attach pid=%lu", (unsigned long)GetCurrentProcessId());


        DisableThreadLibraryCalls(hMod);

        ShellExecuteA(NULL, "open", "https://t.me/ze0ntap", NULL, NULL, SW_SHOWNORMAL);

        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);

        CreateThread(nullptr, 0, HackThread, hMod, 0, nullptr);

        CreateThread(nullptr, 0, KeyListenerThread, hMod, 0, nullptr);

        CreateThread(nullptr, 0, TrailThread, hMod, 0, nullptr);

        break;

    case DLL_PROCESS_DETACH:

        kiero::shutdown();

        MH_Uninitialize();

        break;

    }

    return TRUE;

}