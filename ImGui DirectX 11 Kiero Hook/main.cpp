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

using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
ResizeBuffersFn oResizeBuffers = nullptr;

HWND window = NULL;

WNDPROC oWndProc;

ID3D11Device* pDevice = NULL;

ID3D11DeviceContext* pContext = NULL;

ID3D11RenderTargetView* mainRenderTargetView;



bool init = false;

bool keyValidated = true;

bool unloadRequested = false;

bool menuOpen = true;

static void ReleaseRenderTarget() {
    if (mainRenderTargetView) {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }
}

static bool CreateRenderTarget(IDXGISwapChain* swapChain) {
    if (!swapChain || !pDevice) return false;
    ID3D11Texture2D* backBuffer = nullptr;
    const HRESULT getResult = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(getResult) || !backBuffer) return false;
    const HRESULT viewResult = pDevice->CreateRenderTargetView(backBuffer, nullptr, &mainRenderTargetView);
    backBuffer->Release();
    return SUCCEEDED(viewResult) && mainRenderTargetView != nullptr;
}

HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT format, UINT flags) {
    ReleaseRenderTarget();
    const HRESULT result = oResizeBuffers(swapChain, bufferCount, width, height, format, flags);
    if (SUCCEEDED(result)) CreateRenderTarget(swapChain);
    return result;
}



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

float menuColor[3] = { 0.066f, 0.059f, 0.141f }; // Fatality window_bg

float accentColor[3] = { 0.761f, 0.09f, 0.314f }; // Fatality selection

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

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);

}



LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (uMsg == WM_KEYUP && wParam == VK_INSERT) {
        menuOpen = !menuOpen;
        if (!menuOpen) {
            showConfigMenu = false;
            waitingForBind = nullptr;
        }
        return true;
    }

    if (keyValidated && menuOpen && ImGui::GetCurrentContext() &&
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;



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

            CreateRenderTarget(pSwapChain);

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            InitImGui();

            init = true;

        }

        else return oPresent(pSwapChain, SyncInterval, Flags);

    }



    if (!mainRenderTargetView && !CreateRenderTarget(pSwapChain))
        return oPresent(pSwapChain, SyncInterval, Flags);

    ImGui_ImplDX11_NewFrame();

    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();







    DrawTrail();

    

    // Render ESP if enabled

    if (boxEsp) {

        BoxEsp();

    }



    // Menu windows are rendered only while Insert toggle is open.
    if (menuOpen) {

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
    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    // Exact palette ported from pavetr1337/fatalImguiMenu (vars::colors).
    const ImVec4 fatal_window_bg(0.066f, 0.059f, 0.141f, 1.0f);
    const ImVec4 fatal_child_bg(0.098f, 0.086f, 0.208f, 1.0f);
    const ImVec4 fatal_text_inactive(1.0f, 1.0f, 1.0f, 0.5f);
    const ImVec4 fatal_selection(0.761f, 0.09f, 0.314f, 1.0f);
    const ImVec4 fatal_selection_high(0.82f, 0.106f, 0.345f, 1.0f);
    const ImVec4 fatal_border(1.0f, 1.0f, 1.0f, 0.10f);
    const ImVec4 fatal_button(0.118f, 0.098f, 0.243f, 1.0f);
    const ImVec4 fatal_button_hover(0.145f, 0.122f, 0.302f, 1.0f);
    const ImVec4 fatal_button_clicked(0.169f, 0.141f, 0.349f, 1.0f);
    const ImVec4 fatal_frame_bg_hover(fatal_window_bg.x * 1.1f, fatal_window_bg.y * 1.1f, fatal_window_bg.z * 1.1f, 1.0f);
    const ImVec4 fatal_frame_bg_active(fatal_frame_bg_hover.x * 1.5f, fatal_frame_bg_hover.y * 1.5f, fatal_frame_bg_hover.z * 1.5f, 1.0f);
    const ImVec4 accent = fatal_selection;

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = fatal_text_inactive;
    style.Colors[ImGuiCol_WindowBg] = fatal_window_bg;
    style.Colors[ImGuiCol_ChildBg] = fatal_child_bg;
    style.Colors[ImGuiCol_PopupBg] = fatal_window_bg;
    style.Colors[ImGuiCol_Border] = fatal_border;
    style.Colors[ImGuiCol_FrameBg] = fatal_window_bg;
    style.Colors[ImGuiCol_FrameBgHovered] = fatal_frame_bg_hover;
    style.Colors[ImGuiCol_FrameBgActive] = fatal_frame_bg_active;
    style.Colors[ImGuiCol_Button] = fatal_button;
    style.Colors[ImGuiCol_ButtonHovered] = fatal_button_hover;
    style.Colors[ImGuiCol_ButtonActive] = fatal_button_clicked;
    style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_HeaderHovered] = fatal_selection;
    style.Colors[ImGuiCol_HeaderActive] = fatal_selection_high;
    style.Colors[ImGuiCol_CheckMark] = fatal_selection;
    style.Colors[ImGuiCol_SliderGrab] = fatal_selection;
    style.Colors[ImGuiCol_SliderGrabActive] = fatal_selection_high;
    style.Colors[ImGuiCol_Separator] = fatal_border;
    style.Colors[ImGuiCol_ScrollbarBg] = fatal_window_bg;
    style.Colors[ImGuiCol_ScrollbarGrab] = fatal_button;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = fatal_button_hover;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = fatal_button_clicked;
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(fatal_selection.x, fatal_selection.y, fatal_selection.z, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(fatal_selection.x, fatal_selection.y, fatal_selection.z, 0.55f);
    style.Colors[ImGuiCol_ResizeGripActive] = fatal_selection_high;

    ImGui::SetNextWindowSize(ImVec2(848.0f, 588.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("ze0nware", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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



    // Real fatalImguiMenu render structure (header/footer/main rows) ported 1:1 in layout math.
    const float maintab_h = ImGui::GetFontSize() * 2.75f;
    const float endtab_h = ImGui::GetFontSize() * 1.5f;
    const ImVec2 winSize = ImGui::GetWindowSize();
    const ImVec2 winPos = ImGui::GetWindowPos();

    static std::vector<std::pair<std::string, ImVec2>> sectionLabels;
    sectionLabels.clear();
    auto AddSectionLabel = [&](const char* label) {
        sectionLabels.emplace_back(label, ImGui::GetCursorScreenPos());
    };
    auto DrawSectionLabels = [&]() {
        for (const auto& item : sectionLabels) {
            const ImVec2 textSize = ImGui::CalcTextSize(item.first.c_str());
            ImGui::GetWindowDrawList()->AddText(ImVec2(item.second.x, item.second.y - textSize.y), ImGui::GetColorU32(ImGuiCol_Text), item.first.c_str());
        }
    };

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.098f, 0.086f, 0.208f, 1.0f));

    // Header row: brand + tabs, exactly like fatal_shapka.
    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    ImGui::BeginChild("fatal_shapka", ImVec2(winSize.x, maintab_h), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPos(ImVec2(14.0f, maintab_h * 0.5f - ImGui::GetFontSize() * 0.6f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.761f, 0.09f, 0.314f, 1.0f));
    ImGui::Text("FATALITY");
    ImGui::PopStyleColor();
    const float header_size = ImGui::CalcTextSize("FATALITY").x;

    const char* tabNames[] = { "MOVEMENT", "VISUALS", "WEAPONS" };
    float prev_b_w = 0.0f;
    for (int tab = 0; tab < 3; ++tab) {
        const ImVec2 buttonSize(ImGui::CalcTextSize(tabNames[tab]).x + 20.0f, ImGui::CalcTextSize(tabNames[tab]).y + 10.0f);
        ImGui::SetCursorPos(ImVec2(header_size + 28.0f + prev_b_w, maintab_h * 0.5f - buttonSize.y * 0.5f));
        const bool selected = currentTab == tab;
        if (!selected) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        if (ImGui::Button(tabNames[tab], buttonSize)) currentTab = tab;
        if (!selected) ImGui::PopStyleColor();
        prev_b_w += buttonSize.x + 10.0f;
    }
    ImGui::SetCursorPos(ImVec2(winSize.x - 96.0f, maintab_h * 0.5f - 15.0f));
    if (ImGui::Button("CONFIG", ImVec2(82.0f, 30.0f))) showConfigMenu = !showConfigMenu;
    ImGui::EndChild();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(winPos.x, winPos.y + maintab_h - 1.0f), ImVec2(winPos.x + winSize.x, winPos.y + maintab_h + 1.0f), ImGui::GetColorU32(ImGuiCol_Border));

    // Footer row: exactly like fatal_footer.
    ImGui::SetCursorPos(ImVec2(0.0f, winSize.y - endtab_h));
    ImGui::BeginChild("fatal_footer", ImVec2(winSize.x, endtab_h), ImGuiChildFlags_None);
    ImGui::SetCursorPos(ImVec2(14.0f, endtab_h * 0.5f - ImGui::GetFontSize() * 0.5f));
    ImGui::TextDisabled("INSERT menu  |  END unload");
    ImGui::SetCursorPos(ImVec2(winSize.x - 100.0f, endtab_h * 0.5f - 16.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.58f, 0.12f, 0.16f, 0.72f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.16f, 0.20f, 0.90f));
    if (ImGui::Button("UNLOAD", ImVec2(86.0f, 30.0f))) {
        unloadRequested = true;
        keyValidated = false;
    }
    ImGui::PopStyleColor(2);
    ImGui::EndChild();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(winPos.x, winPos.y + winSize.y - endtab_h - 1.0f), ImVec2(winPos.x + winSize.x, winPos.y + winSize.y - endtab_h + 1.0f), ImGui::GetColorU32(ImGuiCol_Border));

    ImGui::PopStyleColor();

    // Main row: exactly like fatal_main -> three bordered columns per tab, section labels floated on the border.
    ImGui::SetCursorPos(ImVec2(10.0f, maintab_h + 1.0f + 10.0f));
    ImGui::BeginChild("fatal_main", ImVec2(winSize.x - 20.0f, winSize.y - endtab_h - maintab_h - 22.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    static bool FatalCheckbox(const char* label, bool* value) {
    ImGuiStyle& style = ImGui::GetStyle();
    const float squareSize = ImGui::GetFrameHeight();
    const float avail = ImGui::GetContentRegionAvail().x;
    const ImVec2 rowPos = ImGui::GetCursorScreenPos();
    const ImVec2 boxPos(rowPos.x + avail - squareSize, rowPos.y);

    ImGui::InvisibleButton(label, ImVec2(avail, squareSize));
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();
    if (clicked) *value = !*value;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 boxColor = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    drawList->AddRectFilled(boxPos, ImVec2(boxPos.x + squareSize, boxPos.y + squareSize), boxColor, style.FrameRounding);

    if (*value) {
        const ImU32 checkColor = ImGui::GetColorU32(ImGuiCol_CheckMark);
        const float pad = squareSize * 0.22f;
        const ImVec2 a(boxPos.x + pad, boxPos.y + squareSize * 0.55f);
        const ImVec2 b(boxPos.x + squareSize * 0.42f, boxPos.y + squareSize - pad);
        const ImVec2 c(boxPos.x + squareSize - pad, boxPos.y + pad);
        drawList->AddLine(a, b, checkColor, 2.0f);
        drawList->AddLine(b, c, checkColor, 2.0f);
    }

    drawList->AddText(ImVec2(rowPos.x, rowPos.y + (squareSize - ImGui::GetFontSize()) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), label);
    return clicked;
}

const float row_width = winSize.x / 3.0f - 13.0f;
    const float row_height = winSize.y - endtab_h - maintab_h - 44.0f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.098f, 0.086f, 0.208f, 1.0f));

    auto FeatureBind = [&](const char* label, bool* value, BindConfig& bind, const char* id) {
        FatalCheckbox((std::string(label) + "##" + id).c_str(), value);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f);
        std::string buttonLabel = waitingForBind == &bind.key ? "[...]##" : (bind.key > 0 ? GetKeyName(bind.key) + "##" : "none##");
        buttonLabel += id;
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(66.0f, 0.0f))) waitingForBind = &bind.key;
        ImGui::Text("toggle");
        ImGui::SameLine();
        FatalCheckbox((std::string("##toggle_") + id).c_str(), &bind.toggleMode);
    };

    if (currentTab == 0) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild("move_row_1", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("GENERAL");
        FeatureBind("Pixel surf", &pixelSurf, psBind, "ps");
        FeatureBind("Jump bug", &jbActive, jbBind, "jb");
        if (jbActive) ImGui::SliderFloat("Surf speed", &surfSpeed, 0.5f, 3.0f, "%.2f");
        ImGui::EndChild();

        ImGui::SameLine(row_width + 10.0f);
        ImGui::BeginChild("move_row_2", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("AIR");
        FeatureBind("Air jump", &airJump, airJumpBind, "aj");
        ImGui::EndChild();

        ImGui::SameLine((row_width + 10.0f) * 2.0f);
        ImGui::BeginChild("move_row_3", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("STATUS");
        ImGui::Text("Pixel surf"); ImGui::SameLine(); ImGui::TextColored(pixelSurf ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), pixelSurf ? "active" : "off");
        ImGui::Text("Jump bug"); ImGui::SameLine(); ImGui::TextColored(jbActive ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), jbActive ? "active" : "off");
        ImGui::Text("Air jump"); ImGui::SameLine(); ImGui::TextColored(airJump ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), airJump ? "active" : "off");
        ImGui::EndChild();

        ImGui::PopStyleVar();
    } else if (currentTab == 1) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild("visual_row_1", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("INDICATORS");
        FeatureBind("Velocity", &showVelocity, velocityBind, "vel");
        FeatureBind("Movement trail", &showTrail, trailBind, "trail");
        if (showTrail) {
            ImGui::SliderInt("Trail length", &maxTrailPoints, 100, 1000);
            ImGui::SliderFloat("Trail distance", &trailMinDistance, 1.0f, 20.0f, "%.1f");
            ImGui::TextDisabled("points: %d", (int)trailPoints.size());
        }
        ImGui::EndChild();

        ImGui::SameLine(row_width + 10.0f);
        ImGui::BeginChild("visual_row_2", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("ESP");
        FeatureBind("Box ESP", &boxEsp, espBind, "esp");
        if (boxEsp) {
            ImGui::SliderInt("Count", &espCount, 1, 20);
            ImGui::SliderFloat("Distance", &espMaxDistance, 10.0f, 200.0f, "%.0f");
        }
        ImGui::EndChild();

        ImGui::SameLine((row_width + 10.0f) * 2.0f);
        ImGui::BeginChild("visual_row_3", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("STATUS");
        ImGui::Text("Velocity"); ImGui::SameLine(); ImGui::TextColored(showVelocity ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), showVelocity ? "on" : "off");
        ImGui::Text("Trail"); ImGui::SameLine(); ImGui::TextColored(showTrail ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), showTrail ? "on" : "off");
        ImGui::Text("Box ESP"); ImGui::SameLine(); ImGui::TextColored(boxEsp ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), boxEsp ? "on" : "off");
        ImGui::EndChild();

        ImGui::PopStyleVar();
    } else {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild("weapon_row_1", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("GENERAL");
        FeatureBind("Infinite ammo", &infinityAmmo, ammoBind, "ammo");
        ImGui::EndChild();

        ImGui::SameLine(row_width + 10.0f);
        ImGui::BeginChild("weapon_row_2", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("STATUS");
        ImGui::Text("Infinite ammo"); ImGui::SameLine(); ImGui::TextColored(infinityAmmo ? accent : ImVec4(0.7f,0.7f,0.7f,1.0f), infinityAmmo ? "on" : "off");
        ImGui::EndChild();

        ImGui::SameLine((row_width + 10.0f) * 2.0f);
        ImGui::BeginChild("weapon_row_3", ImVec2(row_width, row_height), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
        AddSectionLabel("INFO");
        ImGui::TextWrapped("Weapon controls follow the same three-column layout as the other tabs.");
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }

    ImGui::PopStyleColor();
    DrawSectionLabels();
    ImGui::EndChild();

    ImGui::End();

    } // menuOpen



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
            kiero::bind(13, (void**)&oResizeBuffers, hkResizeBuffers);

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