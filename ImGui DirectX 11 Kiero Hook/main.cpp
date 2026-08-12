#include "includes.h"
#include "il2cpp.h"
#include "velocity_limiter.h"
#include "cat_skybox_bytes.h"

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <cstring>
#include <cmath>

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

#include "edge_bug.h"

struct Vector2 {

    float x, y;

    Vector2() : x(0), y(0) {}

    Vector2(float _x, float _y) : x(_x), y(_y) {}

};



struct Vector4 {

    float x, y, z, w;

};



struct Color {

    float r, g, b, a;

    Color() : r(0), g(0), b(0), a(1) {}

    Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}

};



struct Matrix16 {

    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;

};



// CharacterController methods

#define OFFSET_CHARACTERCONTROLLER_GET_ISGROUNDED  0x2F4DEB0

#define OFFSET_CHARACTERCONTROLLER_MOVE            0x2F4DD80

#define OFFSET_CHARACTERCONTROLLER_GET_VELOCITY    0x2F4DF80

#define OFFSET_TRANSFORM_GET_POSITION              0x2F06CE0
#define OFFSET_TRANSFORM_GET_FORWARD               0x2F06780

#define OFFSET_COMPONENT_GET_TRANSFORM             0x2EF2D50

#define OFFSET_GET_PLAYERCONTROLLER                0x83F820  // PlayerManager.qjm() -> dcvh (current player controller)

#define OFFSET_CAMERA_MAIN                         0x2EC1090

#define OFFSET_CAMERA_SET_FIELDOFVIEW              0x2EC1A80

#define OFFSET_CAMERA_SET_BACKGROUNDCOLOR          0x2EC1890

#define OFFSET_CAMERA_SET_CLEARFLAGS               0x2EC1920

#define OFFSET_TEXTURE2D_CTOR                      0x2EDB780
#define OFFSET_IMAGECONVERSION_LOADIMAGE           0x2F2B980
#define OFFSET_SHADER_FIND                         0x2ED9DD0
#define OFFSET_MATERIAL_CTOR                       0x2ECEA40
#define OFFSET_MATERIAL_SET_MAINTEXTURE            0x2ECF320
#define OFFSET_RENDERER_GET_MATERIAL                0x2ED8EA0
#define OFFSET_RENDERER_SET_MATERIAL                0x2ED9230
#define OFFSET_MATERIAL_GET_COLOR                   0x2ECEBB0
#define OFFSET_MATERIAL_SET_COLOR                   0x2ECF040
#define OFFSET_MATERIAL_SET_RENDERQUEUE             0x2ECF3F0
#define OFFSET_RENDERSETTINGS_GET_SKYBOX            0x2ED8D60
#define OFFSET_RENDERSETTINGS_SET_SKYBOX            0x2ED8D90
#define OFFSET_GUNCONTROLLER_FIRE                  0x996BE0
#define OFFSET_HITCASTER_CAST                      0x99FBB0
#define OFFSET_AIMVIEW_AWAKE                       0xA60AA0
#define OFFSET_AIMVIEW_UPDATE_SNIPER_PANELS         0xA61960
#define OFFSET_HUDVIEW_UPDATE                       0xA68C60
#define OFFSET_COMPONENT_GET_GAMEOBJECT             0x2EF2CA0
#define OFFSET_GAMEOBJECT_SETACTIVE                 0x2EF6620
#define OFFSET_GAMEOBJECT_GET_ACTIVEINHIERARCHY     0x2EF6A20
#define OFFSET_CANVASGROUP_SET_ALPHA                0x312A590

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

BindConfig edgeBugBind = { 0, false };

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

bool edgeBugEnabled = false;
float edgeBugPullForce = 20.0f;

bool showVelocity = false;

bool showTrail = false;

bool cameraFovEnabled = false;
float cameraFov = 90.0f;
bool customSkyboxEnabled = false;
float customSkyboxColor[3] = { 0.15f, 0.25f, 0.55f };
bool imageSkyboxEnabled = false;
char imageSkyboxUrl[512] = "";
char imageSkyboxStatus[128] = "Paste a direct JPG/PNG URL";
uintptr_t imageSkyboxTexture = 0;
uintptr_t imageSkyboxMaterial = 0;
uint32_t imageSkyboxTextureHandle = 0;
uint32_t imageSkyboxMaterialHandle = 0;
uintptr_t originalSkyboxMaterial = 0;
volatile LONG pendingSkyboxCommand = 0;

bool infinityAmmo = false;
bool bulletTracerEnabled = false;
bool noSpreadEnabled = false;
bool removeScopeBorders = false;
bool customScopeReticleVisible = false;
uintptr_t liveAimView = 0;
uintptr_t sniperSightObject = 0;
volatile LONG pendingScopeOverlayRefresh = 0;
float bulletTracerStartColor[3] = { 1.0f, 0.15f, 0.05f };
float bulletTracerEndColor[3] = { 0.15f, 0.55f, 1.0f };
float bulletTracerDuration = 1.5f;
float bulletTracerThickness = 2.5f;

bool weaponChamsEnabled = false;
int weaponChamsMode = 0; // 0 = Flat, 1 = Glass
float weaponChamsColor[3] = { 0.05f, 0.75f, 1.0f };
float weaponChamsGlassAlpha = 0.35f;

struct WeaponChamsRenderer {
    uintptr_t renderer;
    uintptr_t originalMaterial;
};
std::vector<WeaponChamsRenderer> weaponChamsRenderers;
uintptr_t weaponChamsController = 0;
uintptr_t activeLocalGunController = 0;
uintptr_t flatChamsMaterial = 0;
uintptr_t glassChamsMaterial = 0;
uint32_t flatChamsMaterialHandle = 0;
uint32_t glassChamsMaterialHandle = 0;
char weaponChamsStatus[128] = "Select Flat or Glass";

struct BulletTracerEntry {
    Vector3 start;
    Vector3 end;
    ULONGLONG createdAt;
};
std::vector<BulletTracerEntry> bulletTracers;
SRWLOCK bulletTracerLock = SRWLOCK_INIT;



float lastSpeed = 0.0f;

bool wasSurfing = false;

float surfSpeed = 1.5f;

// Velocity limiter
bool velocityLimiterEnabled = false;
float velocityLimit = 500.0f;



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

float trailMinDistance = 0.05f; // Unity units are meters; sample every 5 cm



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
Vector3(__fastcall* o_Transform_get_forward)(uintptr_t) = nullptr;

uintptr_t(__fastcall* o_Component_get_transform)(uintptr_t) = nullptr;

uintptr_t lastCharacterController = 0;

uintptr_t(__fastcall* o_GetPlayerController)() = nullptr;

uintptr_t(__fastcall* o_Camera_get_main)() = nullptr;

void(__fastcall* o_Camera_set_fieldOfView)(uintptr_t, float) = nullptr;

void(__fastcall* o_Camera_set_backgroundColor)(uintptr_t, Color) = nullptr;

void(__fastcall* o_Camera_set_clearFlags)(uintptr_t, int) = nullptr;

void(__fastcall* o_Texture2D_ctor)(uintptr_t, int, int, const Il2CppMethod*) = nullptr;
bool(__fastcall* o_ImageConversion_LoadImage)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Shader_Find)(Il2CppString*, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Material_ctor)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Material_set_mainTexture)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Renderer_get_material)(uintptr_t) = nullptr;
void(__fastcall* o_Renderer_set_material)(uintptr_t, uintptr_t) = nullptr;
Color(__fastcall* o_Material_get_color)(uintptr_t) = nullptr;
void(__fastcall* o_Material_set_color)(uintptr_t, Color) = nullptr;
void(__fastcall* o_Material_set_renderQueue)(uintptr_t, int) = nullptr;
uintptr_t(__fastcall* o_RenderSettings_get_skybox)(const Il2CppMethod*) = nullptr;
void(__fastcall* o_RenderSettings_set_skybox)(uintptr_t, const Il2CppMethod*) = nullptr;

Vector3(__fastcall* o_WorldToScreenPoint)(uintptr_t, Vector3) = nullptr;

Matrix16(__fastcall* o_Camera_get_worldToCameraMatrix)(uintptr_t) = nullptr;

Matrix16(__fastcall* o_Camera_get_projectionMatrix)(uintptr_t) = nullptr;

short(__fastcall* o_GunController_GetCurrentAmmo)(uintptr_t) = nullptr;
void(__fastcall* o_GunController_Fire)(uintptr_t, Vector3, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_HitCaster_Cast)(Vector3, Vector3, float, uintptr_t, const Il2CppMethod*) = nullptr;
thread_local bool insideLocalGunFire = false;
void(__fastcall* o_AimView_Awake)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_AimView_UpdateSniperPanels)(uintptr_t, float, float, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Component_get_gameObject)(uintptr_t) = nullptr;
void(__fastcall* o_GameObject_SetActive)(uintptr_t, bool) = nullptr;
bool(__fastcall* o_GameObject_get_activeInHierarchy)(uintptr_t) = nullptr;
void(__fastcall* o_CanvasGroup_set_alpha)(uintptr_t, float) = nullptr;
void(__fastcall* o_HUDView_Update)(uintptr_t, const Il2CppMethod*) = nullptr;



uintptr_t GetCamera() {

    if (!base) return 0;

    __try { return o_Camera_get_main(); }

    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

}



void __fastcall hk_Camera_set_fieldOfView(uintptr_t camera, float value) {

    if (keyValidated && cameraFovEnabled) value = cameraFov;

    o_Camera_set_fieldOfView(camera, value);

}



void ApplyCameraFov() {

    if (!cameraFovEnabled || !o_Camera_set_fieldOfView) return;

    const uintptr_t camera = GetCamera();

    if (!camera) return;

    __try { o_Camera_set_fieldOfView(camera, cameraFov); }

    __except (EXCEPTION_EXECUTE_HANDLER) {}

}



static bool InvokeVoid(const Il2CppMethod* method, void* object, void** args, const char* failure) {
    if (!method || !g_il2cpp.runtime_invoke) {
        strcpy_s(imageSkyboxStatus, failure);
        return false;
    }
    Il2CppObject* exception = nullptr;
    g_il2cpp.runtime_invoke(method, object, args, &exception);
    if (exception) {
        strcpy_s(imageSkyboxStatus, failure);
        return false;
    }
    return true;
}

bool BuildInternetSkyboxUnityObjects(uintptr_t texture, uintptr_t data) {
    if (!g_il2cpp.class_get_method_from_name || !g_il2cpp.runtime_invoke || !g_il2cpp.object_unbox || !g_il2cpp.thread_attach) {
        strcpy_s(imageSkyboxStatus, "IL2CPP invoke API unavailable");
        return false;
    }

    Il2CppDomain domain = g_il2cpp.domain_get();
    if (!domain || !g_il2cpp.thread_attach(domain)) {
        strcpy_s(imageSkyboxStatus, "Could not attach render thread to IL2CPP");
        return false;
    }

    Il2CppClass* textureClass = g_il2cpp.find_class("UnityEngine", "Texture2D");
    Il2CppClass* imageClass = g_il2cpp.find_class("UnityEngine", "ImageConversion");
    Il2CppClass* shaderClass = g_il2cpp.find_class("UnityEngine", "Shader");
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    Il2CppClass* renderSettingsClass = g_il2cpp.find_class("UnityEngine", "RenderSettings");
    if (!textureClass || !imageClass || !shaderClass || !materialClass || !renderSettingsClass) {
        strcpy_s(imageSkyboxStatus, "Required Unity class not found");
        return false;
    }

    const Il2CppMethod* textureCtor = g_il2cpp.class_get_method_from_name(textureClass, ".ctor", 5);
    const Il2CppMethod* loadImage = g_il2cpp.class_get_method_from_name(imageClass, "LoadImage", 2);
    const Il2CppMethod* shaderFind = g_il2cpp.class_get_method_from_name(shaderClass, "Find", 1);
    const Il2CppMethod* setMainTexture = g_il2cpp.class_get_method_from_name(materialClass, "set_mainTexture", 1);
    const Il2CppMethod* getSkybox = g_il2cpp.class_get_method_from_name(renderSettingsClass, "get_skybox", 0);
    const Il2CppMethod* setSkybox = g_il2cpp.class_get_method_from_name(renderSettingsClass, "set_skybox", 1);

    int width = 2, height = 2;
    int textureFormat = 4; // TextureFormat.RGBA32
    int mipCount = 1;
    bool linear = false;
    void* ctorArgs[] = { &width, &height, &textureFormat, &mipCount, &linear };
    if (!InvokeVoid(textureCtor, reinterpret_cast<void*>(texture), ctorArgs, "Texture2D constructor failed")) return false;

    void* imageArgs[] = { reinterpret_cast<void*>(texture), reinterpret_cast<void*>(data) };
    Il2CppObject* exception = nullptr;
    Il2CppObject* loadResult = g_il2cpp.runtime_invoke(loadImage, nullptr, imageArgs, &exception);
    if (exception || !loadResult || !*reinterpret_cast<bool*>(g_il2cpp.object_unbox(loadResult))) {
        strcpy_s(imageSkyboxStatus, "ImageConversion.LoadImage failed");
        return false;
    }

    Il2CppString* shaderName = g_il2cpp.string_new("Skybox/Panoramic");
    void* shaderArgs[] = { shaderName };
    exception = nullptr;
    Il2CppObject* shaderObject = g_il2cpp.runtime_invoke(shaderFind, nullptr, shaderArgs, &exception);
    if (exception || !shaderObject) {
        strcpy_s(imageSkyboxStatus, "Skybox/Panoramic shader not found");
        return false;
    }

    uintptr_t material = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(materialClass));
    if (!material) {
        strcpy_s(imageSkyboxStatus, "Material allocation failed");
        return false;
    }

    // Material has Shader, Material and string constructors with one argument.
    // Use the exact dumped Material(Shader) RVA instead of ambiguous name/count lookup.
    __try {
        o_Material_ctor(material, reinterpret_cast<uintptr_t>(shaderObject), nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(imageSkyboxStatus, "Material(Shader) constructor failed");
        return false;
    }

    void* textureArgs[] = { reinterpret_cast<void*>(texture) };
    if (!InvokeVoid(setMainTexture, reinterpret_cast<void*>(material), textureArgs, "Material texture assignment failed")) return false;

    if (!originalSkyboxMaterial) {
        exception = nullptr;
        originalSkyboxMaterial = reinterpret_cast<uintptr_t>(g_il2cpp.runtime_invoke(getSkybox, nullptr, nullptr, &exception));
    }

    void* skyboxArgs[] = { reinterpret_cast<void*>(material) };
    if (!InvokeVoid(setSkybox, nullptr, skyboxArgs, "RenderSettings.set_skybox failed")) return false;

    if (g_il2cpp.gchandle_free) {
        if (imageSkyboxTextureHandle) g_il2cpp.gchandle_free(imageSkyboxTextureHandle);
        if (imageSkyboxMaterialHandle) g_il2cpp.gchandle_free(imageSkyboxMaterialHandle);
    }
    imageSkyboxTextureHandle = g_il2cpp.gchandle_new ? g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(texture), false) : 0;
    imageSkyboxMaterialHandle = g_il2cpp.gchandle_new ? g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false) : 0;
    imageSkyboxTexture = texture;
    imageSkyboxMaterial = material;
    imageSkyboxEnabled = true;
    customSkyboxEnabled = false;
    if (o_Camera_set_clearFlags) {
        const uintptr_t camera = GetCamera();
        if (camera) o_Camera_set_clearFlags(camera, 1);
    }
    strcpy_s(imageSkyboxStatus, "Image skybox loaded");
    return true;
}

bool LoadEmbeddedCatSkybox() {

    if (!g_il2cpp.thread_attach || !g_il2cpp.domain_get || !g_il2cpp.thread_attach(g_il2cpp.domain_get())) {
        strcpy_s(imageSkyboxStatus, "Could not attach render thread to IL2CPP");
        return false;
    }

    if (!g_il2cpp.object_new || !g_il2cpp.array_new) {
        strcpy_s(imageSkyboxStatus, "IL2CPP allocator unavailable");
        return false;
    }

    Il2CppClass* textureClass = g_il2cpp.find_class("UnityEngine", "Texture2D");
    Il2CppClass* byteClass = g_il2cpp.find_class("System", "Byte");
    if (!textureClass || !byteClass) {
        strcpy_s(imageSkyboxStatus, "Unity Texture2D/Byte class not found");
        return false;
    }

    uintptr_t texture = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(textureClass));
    Il2CppArray* data = g_il2cpp.array_new(byteClass, kCatSkyboxJpgSize);
    if (!texture || !data) {
        strcpy_s(imageSkyboxStatus, "Unity texture allocation failed");
        return false;
    }

    memcpy(reinterpret_cast<unsigned char*>(data) + 0x20, kCatSkyboxJpg, kCatSkyboxJpgSize);
    return BuildInternetSkyboxUnityObjects(texture, reinterpret_cast<uintptr_t>(data));
}

bool LoadInternetSkybox() {

    if (!g_il2cpp.thread_attach || !g_il2cpp.domain_get || !g_il2cpp.thread_attach(g_il2cpp.domain_get())) {
        strcpy_s(imageSkyboxStatus, "Could not attach render thread to IL2CPP");
        return false;
    }

    if (!imageSkyboxUrl[0] || !g_il2cpp.object_new || !g_il2cpp.array_new || !g_il2cpp.string_new) {
        strcpy_s(imageSkyboxStatus, "Enter a direct JPG/PNG URL");
        return false;
    }

    char tempPath[MAX_PATH] = {};
    char tempFile[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempPath) || !GetTempFileNameA(tempPath, "sky", 0, tempFile)) {
        strcpy_s(imageSkyboxStatus, "Could not create temp file");
        return false;
    }

    const HRESULT download = URLDownloadToFileA(nullptr, imageSkyboxUrl, tempFile, 0, nullptr);
    if (FAILED(download)) {
        DeleteFileA(tempFile);
        strcpy_s(imageSkyboxStatus, "Download failed: use a direct HTTPS image URL");
        return false;
    }

    std::ifstream file(tempFile, std::ios::binary | std::ios::ate);
    if (!file) {
        DeleteFileA(tempFile);
        strcpy_s(imageSkyboxStatus, "Could not read downloaded image");
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size <= 0 || size > 20 * 1024 * 1024) {
        file.close();
        DeleteFileA(tempFile);
        strcpy_s(imageSkyboxStatus, "Image must be between 1 byte and 20 MB");
        return false;
    }

    std::vector<unsigned char> bytes((size_t)size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes.data()), size);
    file.close();
    DeleteFileA(tempFile);

    Il2CppClass* textureClass = g_il2cpp.find_class("UnityEngine", "Texture2D");
    Il2CppClass* byteClass = g_il2cpp.find_class("System", "Byte");
    if (!textureClass || !byteClass) {
        strcpy_s(imageSkyboxStatus, "Unity Texture2D/Byte class not found");
        return false;
    }

    uintptr_t texture = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(textureClass));
    Il2CppArray* data = g_il2cpp.array_new(byteClass, bytes.size());
    if (!texture || !data) {
        strcpy_s(imageSkyboxStatus, "Unity texture allocation failed");
        return false;
    }

    memcpy(reinterpret_cast<unsigned char*>(data) + 0x20, bytes.data(), bytes.size());

    return BuildInternetSkyboxUnityObjects(texture, reinterpret_cast<uintptr_t>(data));
}

void RestoreDefaultSkybox() {
    imageSkyboxEnabled = false;
    customSkyboxEnabled = false;
    if (o_RenderSettings_set_skybox && originalSkyboxMaterial) {
        __try { o_RenderSettings_set_skybox(originalSkyboxMaterial, nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(imageSkyboxStatus, "Default skybox restored");
}

void ApplyImageSkybox() {
    if (!imageSkyboxEnabled || !imageSkyboxMaterial || !o_RenderSettings_set_skybox) return;
    __try {
        const uintptr_t camera = GetCamera();
        if (camera && o_Camera_set_clearFlags) o_Camera_set_clearFlags(camera, 1);
        o_RenderSettings_set_skybox(imageSkyboxMaterial, nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

void ApplyCustomSkybox() {

    if (!customSkyboxEnabled || !o_Camera_set_backgroundColor || !o_Camera_set_clearFlags) return;

    const uintptr_t camera = GetCamera();
    if (!camera) return;

    const Color color(customSkyboxColor[0], customSkyboxColor[1], customSkyboxColor[2], 1.0f);

    __try {
        o_Camera_set_clearFlags(camera, 2); // CameraClearFlags.Color
        o_Camera_set_backgroundColor(camera, color);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

}



uintptr_t GetPlayerController() {

    if (!base) return 0;

    __try { return o_GetPlayerController(); }

    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

}



// Hook на GunController::gcloafhgkcb() - возвращает текущие патроны

void __fastcall hk_GameObject_SetActive(uintptr_t instance, bool active)
{
    if (instance && instance == sniperSightObject) customScopeReticleVisible = removeScopeBorders && active;
    o_GameObject_SetActive(instance, active);
}

void __fastcall hk_AimView_Awake(uintptr_t instance, const Il2CppMethod* method)
{
    o_AimView_Awake(instance, method);
    __try {
        liveAimView = instance;
        sniperSightObject = instance ? *(uintptr_t*)(instance + 0x48) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { sniperSightObject = 0; }
}

void __fastcall hk_AimView_UpdateSniperPanels(uintptr_t instance, float a, float b, const Il2CppMethod* method)
{
    // This runs on every aiming transition, so it also captures AimView when injected mid-match.
    __try {
        if (instance) {
            liveAimView = instance;
            sniperSightObject = *(uintptr_t*)(instance + 0x48);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    o_AimView_UpdateSniperPanels(instance, a, b, method);
}

static void ApplyScopeOverlayState()
{
    if (!liveAimView || !sniperSightObject || !o_GameObject_get_activeInHierarchy || !o_CanvasGroup_set_alpha) {
        customScopeReticleVisible = false;
        return;
    }
    __try {
        const bool scoped = o_GameObject_get_activeInHierarchy(sniperSightObject);
        customScopeReticleVisible = removeScopeBorders && scoped;
        const uintptr_t scopeCanvasGroup = *(uintptr_t*)(liveAimView + 0xA8);
        if (scopeCanvasGroup) o_CanvasGroup_set_alpha(scopeCanvasGroup, removeScopeBorders && scoped ? 0.0f : 1.0f);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { customScopeReticleVisible = false; }
}

static void UpdateWeaponChams(uintptr_t knownWeaponController = 0);

void __fastcall hk_HUDView_Update(uintptr_t instance, const Il2CppMethod* method)
{
    __try {
        liveAimView = instance ? *(uintptr_t*)(instance + 0x38) : 0;
        sniperSightObject = liveAimView ? *(uintptr_t*)(liveAimView + 0x48) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { liveAimView = 0; sniperSightObject = 0; }
    o_HUDView_Update(instance, method);
    ApplyScopeOverlayState();
}

void DrawBorderlessScopeReticle()
{
    if (!removeScopeBorders || !customScopeReticleVisible) return;
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const ImVec2 center(display.x * 0.5f, display.y * 0.5f);
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    const ImU32 shadow = IM_COL32(255, 255, 255, 90);
    const ImU32 line = IM_COL32(0, 0, 0, 235);
    draw->AddLine(ImVec2(0.0f, center.y + 1.0f), ImVec2(display.x, center.y + 1.0f), shadow, 1.0f);
    draw->AddLine(ImVec2(center.x + 1.0f, 0.0f), ImVec2(center.x + 1.0f, display.y), shadow, 1.0f);
    draw->AddLine(ImVec2(0.0f, center.y), ImVec2(display.x, center.y), line, 1.0f);
    draw->AddLine(ImVec2(center.x, 0.0f), ImVec2(center.x, display.y), line, 1.0f);
    draw->AddCircleFilled(center, 2.0f, line);
}

uintptr_t __fastcall hk_HitCaster_Cast(Vector3 origin, Vector3 direction, float maxDistance, uintptr_t hitParameters, const Il2CppMethod* method)
{
    if (keyValidated && noSpreadEnabled && insideLocalGunFire) {
        __try {
            const uintptr_t camera = GetCamera();
            const uintptr_t cameraTransform = camera && o_Component_get_transform ? o_Component_get_transform(camera) : 0;
            if (cameraTransform && o_Transform_get_forward)
                direction = o_Transform_get_forward(cameraTransform);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    const uintptr_t result = o_HitCaster_Cast(origin, direction, maxDistance, hitParameters, method);
    if (keyValidated && bulletTracerEnabled && insideLocalGunFire && result) {
        __try {
            // cjr stores the authoritative cast start/end at 0x24/0x30.
            const Vector3 actualStart = *(Vector3*)(result + 0x24);
            const Vector3 actualEnd = *(Vector3*)(result + 0x30);
            AcquireSRWLockExclusive(&bulletTracerLock);
            bulletTracers.push_back({ actualStart, actualEnd, GetTickCount64() });
            if (bulletTracers.size() > 128) bulletTracers.erase(bulletTracers.begin(), bulletTracers.begin() + (bulletTracers.size() - 128));
            ReleaseSRWLockExclusive(&bulletTracerLock);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return result;
}

void __fastcall hk_GunController_Fire(uintptr_t instance, Vector3 playSound, const Il2CppMethod* method)
{
    activeLocalGunController = instance;
    UpdateWeaponChams(instance);
    insideLocalGunFire = true;
    o_GunController_Fire(instance, playSound, method);
    insideLocalGunFire = false;
}

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



bool UnityWorldToScreen(const Vector3& pos, Vector2& screen) {

    if (!o_WorldToScreenPoint) return false;

    const uintptr_t camera = GetCamera();
    if (!camera) return false;

    __try {
        const Vector3 projected = o_WorldToScreenPoint(camera, pos);
        if (projected.z <= 0.01f) return false;

        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        screen.x = projected.x;
        screen.y = displaySize.y - projected.y;
        return screen.x >= -100.0f && screen.x <= displaySize.x + 100.0f &&
               screen.y >= -100.0f && screen.y <= displaySize.y + 100.0f;
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

    const bool grounded = o_CC_get_isGrounded(instance);
    if (!keyValidated) return grounded;

    if (airJump && instance) return true;

    if (jbActive) return false;

    return grounded;

}



Vector3 __fastcall hk_CC_get_velocity(uintptr_t instance)
{
    Vector3 vel = o_CC_get_velocity(instance);
    if (keyValidated) {
        lastSpeed = sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
        if (jbActive) {
            float horSpeed = sqrt(vel.x * vel.x + vel.z * vel.z);
            if (horSpeed < 1.0f) horSpeed = surfSpeed * 5.0f;
            
            // Original acceleration
            if (vel.Length2D() < surfSpeed * 3.0f) {
                vel.x *= 1.05f;
                vel.z *= 1.05f;
            }
            
            // Apply our new velocity limiter clamp
            vel = ApplyVelocityLimit(vel);
            
            vel.y = 0;
        }
    }
    return vel;
}



static void RestoreWeaponChams()
{
    if (o_Renderer_set_material) {
        for (const WeaponChamsRenderer& entry : weaponChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            __try { o_Renderer_set_material(entry.renderer, entry.originalMaterial); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    weaponChamsRenderers.clear();
    weaponChamsController = 0;
}

static uintptr_t CreateWeaponChamsMaterial(const char* shaderName, bool glass)
{
    if (!shaderName || !o_Shader_Find || !o_Material_ctor || !g_il2cpp.object_new || !g_il2cpp.string_new) return 0;
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) return 0;
    const uintptr_t shader = o_Shader_Find(g_il2cpp.string_new(shaderName), nullptr);
    if (!shader) return 0;
    const uintptr_t material = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(materialClass));
    if (!material) return 0;
    o_Material_ctor(material, shader, nullptr);
    if (glass && o_Material_set_renderQueue) o_Material_set_renderQueue(material, 3000);
    return material;
}

static uintptr_t EnsureSelectedWeaponChamsMaterial()
{
    if (weaponChamsMode == 0) {
        if (!flatChamsMaterial) {
            const char* flatShaders[] = { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default" };
            for (const char* shaderName : flatShaders) {
                flatChamsMaterial = CreateWeaponChamsMaterial(shaderName, false);
                if (flatChamsMaterial) break;
            }
            if (flatChamsMaterial && g_il2cpp.gchandle_new)
                flatChamsMaterialHandle = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(flatChamsMaterial), false);
        }
        if (!flatChamsMaterial) strcpy_s(weaponChamsStatus, "Flat shader not found in this build");
        return flatChamsMaterial;
    }

    if (!glassChamsMaterial) {
        const char* glassShaders[] = { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default" };
        for (const char* shaderName : glassShaders) {
            glassChamsMaterial = CreateWeaponChamsMaterial(shaderName, true);
            if (glassChamsMaterial) break;
        }
        if (glassChamsMaterial && g_il2cpp.gchandle_new)
            glassChamsMaterialHandle = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(glassChamsMaterial), false);
    }
    if (!glassChamsMaterial) strcpy_s(weaponChamsStatus, "Glass shader not found in this build");
    return glassChamsMaterial;
}

static void CaptureWeaponChamsRenderers(uintptr_t weaponController)
{
    RestoreWeaponChams();
    if (!weaponController || !o_Renderer_get_material) {
        strcpy_s(weaponChamsStatus, "No local weapon controller");
        return;
    }
    __try {
        const uintptr_t lodGroup = *(uintptr_t*)(weaponController + 0x80);
        const uintptr_t renderers = lodGroup ? *(uintptr_t*)(lodGroup + 0x40) : 0;
        if (!lodGroup) { strcpy_s(weaponChamsStatus, "WeaponLodGroup not found"); return; }
        if (!renderers) { strcpy_s(weaponChamsStatus, "Weapon renderer array not found"); return; }
        const size_t count = *(size_t*)(renderers + 0x18);
        if (!count || count > 64) { strcpy_s(weaponChamsStatus, "Weapon renderer array is empty"); return; }
        for (size_t i = 0; i < count; ++i) {
            const uintptr_t renderer = *(uintptr_t*)(renderers + 0x20 + i * sizeof(uintptr_t));
            if (!renderer) continue;
            const uintptr_t originalMaterial = o_Renderer_get_material(renderer);
            if (originalMaterial) weaponChamsRenderers.push_back({ renderer, originalMaterial });
        }
        weaponChamsController = weaponController;
        sprintf_s(weaponChamsStatus, "Applied to %zu weapon renderer(s)", weaponChamsRenderers.size());
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(weaponChamsStatus, "Weapon renderer capture failed");
        RestoreWeaponChams();
    }
}

static void UpdateWeaponChams(uintptr_t knownWeaponController)
{
    if (!keyValidated) { strcpy_s(weaponChamsStatus, "License validation inactive"); return; }
    if (!o_Renderer_set_material || !o_Material_set_color) { strcpy_s(weaponChamsStatus, "Material API unavailable"); return; }
    uintptr_t weaponController = knownWeaponController ? knownWeaponController : activeLocalGunController;
    if (!weaponController) {
        __try {
            const uintptr_t localPlayer = GetPlayerController();
            const uintptr_t weaponry = localPlayer ? *(uintptr_t*)(localPlayer + OFFSET_WEAPONRYCONTROLLER) : 0;
            weaponController = weaponry ? *(uintptr_t*)(weaponry + OFFSET_WEAPONCONTROLLER) : 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { weaponController = 0; }
    }

    if (!weaponChamsEnabled) {
        if (!weaponChamsRenderers.empty()) RestoreWeaponChams();
        strcpy_s(weaponChamsStatus, "Disabled");
        return;
    }
    if (!weaponController) {
        strcpy_s(weaponChamsStatus, "Fire once to capture active weapon");
        return;
    }
    const uintptr_t replacement = EnsureSelectedWeaponChamsMaterial();
    if (!replacement) return;
    if (weaponController != weaponChamsController || weaponChamsRenderers.empty())
        CaptureWeaponChamsRenderers(weaponController);
    if (weaponChamsRenderers.empty()) return;

    const float alpha = weaponChamsMode == 1 ? weaponChamsGlassAlpha : 1.0f;
    o_Material_set_color(replacement, Color(weaponChamsColor[0], weaponChamsColor[1], weaponChamsColor[2], alpha));
    for (const WeaponChamsRenderer& entry : weaponChamsRenderers) {
        if (!entry.renderer) continue;
        __try { o_Renderer_set_material(entry.renderer, replacement); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}

int __fastcall hk_CC_Move(uintptr_t instance, Vector3 motion)
{
    if (keyValidated && instance) lastCharacterController = instance;

    if (activeLocalGunController) UpdateWeaponChams(activeLocalGunController);

    if (InterlockedExchange(&pendingScopeOverlayRefresh, 0)) ApplyScopeOverlayState();

    const LONG skyboxCommand = InterlockedExchange(&pendingSkyboxCommand, 0);
    if (keyValidated && skyboxCommand == 1) LoadEmbeddedCatSkybox();
    else if (keyValidated && skyboxCommand == 2) LoadInternetSkybox();
    else if (keyValidated && skyboxCommand == 3) RestoreDefaultSkybox();

    if (keyValidated && jbActive) {
        float speed = sqrt(motion.x * motion.x + motion.z * motion.z);
        if (speed > 0.1f) {
            motion.x = (motion.x / speed) * surfSpeed * 10.0f;
            motion.z = (motion.z / speed) * surfSpeed * 10.0f;
        }
        
        // If limiter is enabled, we must also clamp the motion vector.
        // Since motion is per-frame, we scale the limit by a factor (e.g. 0.016 for 60fps)
        // But to be safe and match the user's slider (which is likely in velocity units),
        // we can just clamp it using the same ApplyVelocityLimit but scaled for motion.
        // Actually, the original code forces motion to surfSpeed * 10.0f.
        // Let's just clamp motion directly if it exceeds the limit * 0.02f (approx frame time).
        if (velocityLimiterEnabled && velocityLimit > 0.0f) {
            float motionLimit = velocityLimit * 0.02f; // Approximate conversion to per-frame motion
            float currentMotionSpeed = sqrt(motion.x * motion.x + motion.z * motion.z);
            if (currentMotionSpeed > motionLimit) {
                motion.x = (motion.x / currentMotionSpeed) * motionLimit;
                motion.z = (motion.z / currentMotionSpeed) * motionLimit;
            }
        }
        
        motion.y = 0;
    }

    if (keyValidated) {
        motion = EdgeBug::ApplyDownwardPull(motion, edgeBugEnabled, edgeBugPullForce);
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

    

    Vector3 pos;
    bool hasPosition = false;

    if (lastCharacterController && o_Component_get_transform && o_Transform_get_position) {
        __try {
            const uintptr_t transform = o_Component_get_transform(lastCharacterController);
            if (transform) {
                pos = o_Transform_get_position(transform);
                hasPosition = true;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lastCharacterController = 0;
        }
    }

    if (!hasPosition) {
        void* localPC = GetLocalPC();
        if (localPC) hasPosition = GetPCPosition(localPC, pos);
    }

    if (!hasPosition) return;

    

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


static bool ProjectTracerEnd(const Vector3& pos, ImVec2& screen) {
    if (!o_WorldToScreenPoint) return false;
    const uintptr_t camera = GetCamera();
    if (!camera) return false;
    __try {
        const Vector3 projected = o_WorldToScreenPoint(camera, pos);
        if (projected.z <= 0.01f) return false;
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        screen = ImVec2(projected.x, display.y - projected.y);
        return true; // Keep off-screen coordinates; ImGui clips the line to the viewport.
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void DrawBulletTracers() {
    if (!keyValidated || !bulletTracerEnabled || !o_WorldToScreenPoint) return;

    const ULONGLONG now = GetTickCount64();
    std::vector<BulletTracerEntry> snapshot;
    AcquireSRWLockExclusive(&bulletTracerLock);
    bulletTracers.erase(std::remove_if(bulletTracers.begin(), bulletTracers.end(), [now](const BulletTracerEntry& tracer) {
        return now - tracer.createdAt > static_cast<ULONGLONG>(bulletTracerDuration * 1000.0f);
    }), bulletTracers.end());
    snapshot = bulletTracers;
    ReleaseSRWLockExclusive(&bulletTracerLock);

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    drawList->PushClipRect(ImVec2(0.0f, 0.0f), display, true);
    for (const BulletTracerEntry& tracer : snapshot) {
        ImVec2 startScreen, endScreen;
        if (!ProjectTracerEnd(tracer.end, endScreen)) continue;

        // Keep the start in world space. If the muzzle is inside the camera near plane,
        // advance along the actual bullet segment until projection becomes valid.
        Vector3 visibleStart = tracer.start;
        bool startVisible = ProjectTracerEnd(visibleStart, startScreen);
        for (int step = 1; !startVisible && step <= 12; ++step) {
            const float t = static_cast<float>(step) / 12.0f;
            visibleStart = Vector3(
                tracer.start.x + (tracer.end.x - tracer.start.x) * t,
                tracer.start.y + (tracer.end.y - tracer.start.y) * t,
                tracer.start.z + (tracer.end.z - tracer.start.z) * t);
            startVisible = ProjectTracerEnd(visibleStart, startScreen);
        }
        if (!startVisible) continue;
        float age = static_cast<float>(now - tracer.createdAt) / (bulletTracerDuration * 1000.0f);
        float alpha = 1.0f - (age < 0.0f ? 0.0f : (age > 1.0f ? 1.0f : age));
        drawList->AddLine(startScreen, endScreen, IM_COL32(0, 0, 0, static_cast<int>(170 * alpha)), bulletTracerThickness + 2.0f);

        // Segment the actual world-space shot path so the color interpolation stays attached
        // to the tracer under perspective instead of becoming a flat screen-space overlay.
        constexpr int gradientSegments = 32;
        ImVec2 previous = startScreen;
        for (int segment = 1; segment <= gradientSegments; ++segment) {
            const float t = static_cast<float>(segment) / static_cast<float>(gradientSegments);
            const Vector3 worldPoint(
                visibleStart.x + (tracer.end.x - visibleStart.x) * t,
                visibleStart.y + (tracer.end.y - visibleStart.y) * t,
                visibleStart.z + (tracer.end.z - visibleStart.z) * t);
            ImVec2 current;
            if (!ProjectTracerEnd(worldPoint, current)) continue;
            const float mid = (static_cast<float>(segment) - 0.5f) / static_cast<float>(gradientSegments);
            const float r = bulletTracerStartColor[0] + (bulletTracerEndColor[0] - bulletTracerStartColor[0]) * mid;
            const float g = bulletTracerStartColor[1] + (bulletTracerEndColor[1] - bulletTracerStartColor[1]) * mid;
            const float b = bulletTracerStartColor[2] + (bulletTracerEndColor[2] - bulletTracerStartColor[2]) * mid;
            const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, alpha));
            drawList->AddLine(previous, current, color, bulletTracerThickness);
            previous = current;
        }
    }
    drawList->PopClipRect();
}

void DrawTrail() {

    if (!keyValidated || !showTrail || trailPoints.empty()) return;

    if (!unityPlayerBase || !o_WorldToScreenPoint) return;

    

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    

    for (size_t i = 0; i + 1 < trailPoints.size(); i++) {

        Vector2 screen1, screen2;

        

        if (UnityWorldToScreen(trailPoints[i], screen1) &&

            UnityWorldToScreen(trailPoints[i + 1], screen2)) {

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

        if (UnityWorldToScreen(trailPoints.back(), currentScreen)) {

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

            if (edgeBugBind.key > 0) {

                bool held = (GetAsyncKeyState(edgeBugBind.key) & 0x8000) != 0;

                static bool edgeBugToggle = false, edgeBugLast = false;

                if (edgeBugBind.toggleMode) { if (held && !edgeBugLast) edgeBugToggle = !edgeBugToggle; edgeBugEnabled = edgeBugToggle; }

                else edgeBugEnabled = held;

                edgeBugLast = held;

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



static bool DrawSmallCheckbox(const char* strId, bool value) {
    static std::unordered_map<ImGuiID, float> animState;
    const float switchW = 34.0f;
    const float switchH = 18.0f;
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const ImGuiID id = ImGui::GetID(strId);
    ImGui::InvisibleButton(strId, ImVec2(switchW, switchH));
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    float& anim = animState[id];
    const float target = value ? 1.0f : 0.0f;
    const float speed = ImGui::GetIO().DeltaTime * 8.0f;
    anim += (target - anim) * (speed > 1.0f ? 1.0f : speed);
    if (fabsf(anim - target) < 0.01f) anim = target;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec4 accentCol = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
    const ImVec4 offCol = ImVec4(1.0f, 1.0f, 1.0f, hovered ? 0.16f : 0.10f);
    const ImVec4 trackCol = ImVec4(
        offCol.x + (accentCol.x - offCol.x) * anim,
        offCol.y + (accentCol.y - offCol.y) * anim,
        offCol.z + (accentCol.z - offCol.z) * anim,
        offCol.w + (accentCol.w - offCol.w) * anim);

    // Glow when on.
    if (anim > 0.02f) {
        for (int i = 3; i >= 1; --i) {
            const float expand = (float)i * 1.8f;
            const float alpha = 0.10f * (4 - i) * anim;
            dl->AddRectFilled(ImVec2(p.x - expand, p.y - expand), ImVec2(p.x + switchW + expand, p.y + switchH + expand),
                ImGui::GetColorU32(ImVec4(accentCol.x, accentCol.y, accentCol.z, alpha)), (switchH * 0.5f) + expand);
        }
    }

    dl->AddRectFilled(p, ImVec2(p.x + switchW, p.y + switchH), ImGui::GetColorU32(trackCol), switchH * 0.5f);

    const float knobR = switchH * 0.5f - 2.0f;
    const float knobX = p.x + switchH * 0.5f + (switchW - switchH) * anim;
    const float knobY = p.y + switchH * 0.5f;
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR, IM_COL32(245, 245, 250, 255), 16);

    return clicked;
}

// Modern card-style feature row: label left, animated toggle switch right, rounded hover background.
static void FeatureLine(const char* displayName, const char* strId, bool* value) {
    const float rowH = 32.0f;
    const ImVec2 rowStart = ImGui::GetCursorScreenPos();
    const float rowW = ImGui::GetContentRegionAvail().x;

    ImGui::PushID(strId);
    ImGui::InvisibleButton("##row", ImVec2(rowW, rowH));
    const bool rowHovered = ImGui::IsItemHovered();
    const bool rowClicked = ImGui::IsItemClicked();
    ImGui::PopID();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (rowHovered || *value) {
        const ImVec4 accentCol = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
        const float a = *value ? (rowHovered ? 0.14f : 0.09f) : 0.06f;
        dl->AddRectFilled(rowStart, ImVec2(rowStart.x + rowW, rowStart.y + rowH), ImGui::GetColorU32(ImVec4(accentCol.x, accentCol.y, accentCol.z, a)), 6.0f);
    }

    dl->AddText(ImVec2(rowStart.x + 10.0f, rowStart.y + (rowH - ImGui::GetFontSize()) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), displayName);

    ImGui::SetCursorScreenPos(ImVec2(rowStart.x + rowW - 34.0f - 10.0f, rowStart.y + (rowH - 18.0f) * 0.5f));
    if (DrawSmallCheckbox(strId, *value)) *value = !*value;
    ImGui::SetCursorScreenPos(ImVec2(rowStart.x, rowStart.y + rowH));

    if (rowClicked) *value = !*value;
}


HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)

{

    if (unloadRequested) {

        // Просто отключаем меню и хуки, но не крашим игру

        keyValidated = false;

        return oPresent(pSwapChain, SyncInterval, Flags);

    }

    if (keyValidated) {
        ApplyCameraFov();
        ApplyCustomSkybox();
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







    UpdateTrail();
    DrawTrail();

    

    // Render ESP if enabled

    if (boxEsp) {

        BoxEsp();

    }



    // Menu windows are rendered only while Insert toggle is open.
    if (menuOpen) {

    if (showConfigMenu) {
        ImGui::Begin("Config", &showConfigMenu);
        ImGui::Text("Menu Customization");
        ImGui::Separator();
        ImGui::ColorEdit3("Menu Color", menuColor);
        ImGui::ColorEdit3("Accent Color", accentColor);
        ImGui::Checkbox("Custom Background", &useCustomBackground);
        if (useCustomBackground) {
            ImGui::Text("Background Path:");
            ImGui::InputText("##bgpath", backgroundPath, sizeof(backgroundPath));
            if (ImGui::Button("Load Background")) {
                backgroundLoaded = false;
                LoadBackgroundTexture();
            }
            ImGui::SameLine();
            ImGui::TextColored(backgroundLoaded ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), backgroundLoaded ? "Loaded!" : "Not found");
        }
        ImGui::End();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 10.0f);
    style.WindowPadding = ImVec2(0.0f, 0.0f);

    const ImVec4 bgDark(0.04f, 0.02f, 0.08f, 1.0f);
    const ImVec4 bgPanel(0.06f, 0.03f, 0.12f, 1.0f);
    const ImVec4 borderCol(1.0f, 1.0f, 1.0f, 0.12f);
    ImVec4 accent(accentColor[0], accentColor[1], accentColor[2], 1.0f);
    const ImVec4 textDim(1.0f, 1.0f, 1.0f, 0.45f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = textDim;
    style.Colors[ImGuiCol_WindowBg] = bgDark;
    style.Colors[ImGuiCol_ChildBg] = bgPanel;
    style.Colors[ImGuiCol_PopupBg] = bgDark;
    style.Colors[ImGuiCol_Border] = borderCol;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(bgDark.x * 1.3f, bgDark.y * 1.3f, bgDark.z * 1.3f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(accent.x, accent.y, accent.z, 0.22f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(accent.x, accent.y, accent.z, 0.32f);
    style.Colors[ImGuiCol_Button] = ImVec4(bgDark.x * 1.5f, bgDark.y * 1.5f, bgDark.z * 1.5f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(accent.x, accent.y, accent.z, 0.28f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(accent.x, accent.y, accent.z, 0.42f);
    style.Colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.16f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.26f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.38f);
    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = accent;
    style.Colors[ImGuiCol_Separator] = borderCol;

    ImGui::SetNextWindowSize(ImVec2(800.0f, 500.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("STANDMOVEMENT", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    // Setup ImGui style to match the "gui" folder (classic dark/orange cheat menu style)
    // ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.WindowPadding = ImVec2(8.0f, 8.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    
    // Orange accent color from gui.hpp (255, 139, 61)
    accent = ImVec4(1.0f, 0.545f, 0.239f, 1.0f);
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.6f, 0.3f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

    // Top Tabs
    static int currentTab = 0;
    ImGui::BeginChild("TabsPanel", ImVec2(0, 30), true);
    
    // Custom tab buttons
    float tabWidth = (ImGui::GetContentRegionAvail().x - 24.0f) / 4.0f;
    
    bool pushed0 = (currentTab == 0);
    if (pushed0) ImGui::PushStyleColor(ImGuiCol_Button, accent);
    if (ImGui::Button("RAGEBOT", ImVec2(tabWidth, 0))) currentTab = 0;
    if (pushed0) ImGui::PopStyleColor();
    
    ImGui::SameLine();
    bool pushed1 = (currentTab == 1);
    if (pushed1) ImGui::PushStyleColor(ImGuiCol_Button, accent);
    if (ImGui::Button("ANTI-AIM", ImVec2(tabWidth, 0))) currentTab = 1;
    if (pushed1) ImGui::PopStyleColor();
    
    ImGui::SameLine();
    bool pushed2 = (currentTab == 2);
    if (pushed2) ImGui::PushStyleColor(ImGuiCol_Button, accent);
    if (ImGui::Button("VISUALS", ImVec2(tabWidth, 0))) currentTab = 2;
    if (pushed2) ImGui::PopStyleColor();
    
    ImGui::SameLine();
    bool pushed3 = (currentTab == 3);
    if (pushed3) ImGui::PushStyleColor(ImGuiCol_Button, accent);
    if (ImGui::Button("WEAPONS", ImVec2(tabWidth, 0))) currentTab = 3;
    if (pushed3) ImGui::PopStyleColor();
    
    ImGui::EndChild();

    // Main Content Area
    ImGui::BeginChild("ContentPanel", ImVec2(0, 0), true);
    
    if (currentTab == 0) { // RAGEBOT (Movement/Misc)
        ImGui::Columns(2, nullptr, false);
        
        ImGui::BeginChild("Movement", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Movement");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Checkbox("Enable BunnyHop", &jbActive);
        if (jbActive) {
            ImGui::SliderFloat("Speed Multiplier", &surfSpeed, 0.5f, 3.0f, "%.2f");
        }
        
        ImGui::Checkbox("Velocity Limiter", &velocityLimiterEnabled);
        if (velocityLimiterEnabled) {
            ImGui::SliderFloat("Max Speed", &velocityLimit, 0.0f, 1000.0f, "%.0f");
        }
        
        ImGui::Checkbox("Air Jump", &airJump);
        ImGui::Checkbox("Edge Bug", &edgeBugEnabled);
        if (edgeBugEnabled) {
            ImGui::SliderFloat("Edge Bug Pull Force", &edgeBugPullForce, 0.0f, 100.0f, "%.1f");
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Pull Force Value", &edgeBugPullForce, 1.0f, 10.0f, "%.1f")) {
                if (edgeBugPullForce < 0.0f) edgeBugPullForce = 0.0f;
                if (edgeBugPullForce > 100.0f) edgeBugPullForce = 100.0f;
            }
        }
        ImGui::Checkbox("Pixel Surf", &pixelSurf);
        
        ImGui::EndChild();
        
        ImGui::NextColumn();
        
        ImGui::BeginChild("Keybinds", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Keybinds");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("BunnyHop Key");
        if (ImGui::Button(waitingForBind == &jbBind.key ? "[...]" : (jbBind.key > 0 ? GetKeyName(jbBind.key).c_str() : "None##jb"), ImVec2(100, 0))) waitingForBind = &jbBind.key;
        
        ImGui::Text("Air Jump Key");
        if (ImGui::Button(waitingForBind == &airJumpBind.key ? "[...]" : (airJumpBind.key > 0 ? GetKeyName(airJumpBind.key).c_str() : "None##aj"), ImVec2(100, 0))) waitingForBind = &airJumpBind.key;

        ImGui::Text("Edge Bug Key");
        if (ImGui::Button(waitingForBind == &edgeBugBind.key ? "[...]" : (edgeBugBind.key > 0 ? GetKeyName(edgeBugBind.key).c_str() : "None##eb"), ImVec2(100, 0))) waitingForBind = &edgeBugBind.key;
        ImGui::SameLine();
        ImGui::Checkbox("Toggle##eb", &edgeBugBind.toggleMode);
        
        ImGui::EndChild();
        
        ImGui::Columns(1);
    }
    else if (currentTab == 1) { // ANTI-AIM
        ImGui::BeginChild("AntiAim", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Anti-Aim");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Not implemented yet.");
        ImGui::EndChild();
    }
    else if (currentTab == 2) { // VISUALS
        ImGui::BeginChild("Visuals", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Visuals");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Checkbox("Box ESP", &boxEsp);
        ImGui::Checkbox("Show Velocity", &showVelocity);
        ImGui::Checkbox("Show Trail", &showTrail);
        ImGui::Checkbox("Camera FOV", &cameraFovEnabled);
        if (cameraFovEnabled) {
            ImGui::SliderFloat("Camera FOV Slider", &cameraFov, 30.0f, 150.0f, "%.1f");
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Camera FOV Value", &cameraFov, 1.0f, 10.0f, "%.1f")) {
                if (cameraFov < 30.0f) cameraFov = 30.0f;
                if (cameraFov > 150.0f) cameraFov = 150.0f;
            }
        }
        ImGui::Checkbox("Custom Skybox", &customSkyboxEnabled);
        if (customSkyboxEnabled) {
            ImGui::ColorEdit3("Skybox Color", customSkyboxColor);
        }
        if (ImGui::Button("Load Embedded Cat Skybox")) {
            strcpy_s(imageSkyboxStatus, "Queued on Unity game thread...");
            InterlockedExchange(&pendingSkyboxCommand, 1);
        }
        ImGui::InputText("Skybox Image URL", imageSkyboxUrl, sizeof(imageSkyboxUrl));
        if (ImGui::Button("Load Image Skybox")) {
            strcpy_s(imageSkyboxStatus, "Queued on Unity game thread...");
            InterlockedExchange(&pendingSkyboxCommand, 2);
        }
        ImGui::SameLine();
        if (ImGui::Button("Restore Skybox")) InterlockedExchange(&pendingSkyboxCommand, 3);
        ImGui::TextWrapped("%s", imageSkyboxStatus);
        
        ImGui::EndChild();
    }
    else if (currentTab == 3) { // WEAPONS
        ImGui::BeginChild("Weapons", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Weapons");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Checkbox("Infinity Ammo", &infinityAmmo);
        ImGui::Checkbox("No Spread", &noSpreadEnabled);
        if (ImGui::Checkbox("Remove Scope Borders", &removeScopeBorders)) InterlockedExchange(&pendingScopeOverlayRefresh, 1);
        if (ImGui::Checkbox("Weapon Chams", &weaponChamsEnabled))
            strcpy_s(weaponChamsStatus, weaponChamsEnabled ? "Fire once to capture active weapon" : "Disabling...");
        if (weaponChamsEnabled) {
            const char* chamsModes[] = { "Flat", "Glass" };
            if (ImGui::Combo("Chams Material", &weaponChamsMode, chamsModes, IM_ARRAYSIZE(chamsModes))) weaponChamsController = 0;
            ImGui::ColorEdit3("Chams Color", weaponChamsColor);
            if (weaponChamsMode == 1) ImGui::SliderFloat("Glass Alpha", &weaponChamsGlassAlpha, 0.05f, 0.95f, "%.2f");
            ImGui::TextWrapped("Status: %s", weaponChamsStatus);
        }
        ImGui::Checkbox("Bullet Tracer", &bulletTracerEnabled);
        if (bulletTracerEnabled) {
            ImGui::ColorEdit3("Tracer Start Color", bulletTracerStartColor);
            ImGui::ColorEdit3("Tracer End Color", bulletTracerEndColor);
            ImGui::SliderFloat("Tracer Duration", &bulletTracerDuration, 0.1f, 5.0f, "%.1f s");
            ImGui::SliderFloat("Tracer Thickness", &bulletTracerThickness, 1.0f, 8.0f, "%.1f");
        }
        
        ImGui::EndChild();
    }
    
    ImGui::EndChild(); // ContentPanel
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

    DrawBulletTracers();
    DrawBorderlessScopeReticle();

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
    o_Transform_get_forward = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_FORWARD);

    o_Component_get_transform = (uintptr_t(__fastcall*)(uintptr_t))(base + OFFSET_COMPONENT_GET_TRANSFORM);

    o_Camera_get_main = (uintptr_t(__fastcall*)())(base + OFFSET_CAMERA_MAIN);

    MH_CreateHook((LPVOID)(base + OFFSET_CAMERA_SET_FIELDOFVIEW), hk_Camera_set_fieldOfView, (LPVOID*)&o_Camera_set_fieldOfView);

    MH_EnableHook((LPVOID)(base + OFFSET_CAMERA_SET_FIELDOFVIEW));

    o_Camera_set_backgroundColor = (void(__fastcall*)(uintptr_t, Color))(base + OFFSET_CAMERA_SET_BACKGROUNDCOLOR);

    o_Camera_set_clearFlags = (void(__fastcall*)(uintptr_t, int))(base + OFFSET_CAMERA_SET_CLEARFLAGS);

    o_Texture2D_ctor = (void(__fastcall*)(uintptr_t, int, int, const Il2CppMethod*))(base + OFFSET_TEXTURE2D_CTOR);
    o_ImageConversion_LoadImage = (bool(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_IMAGECONVERSION_LOADIMAGE);
    o_Shader_Find = (uintptr_t(__fastcall*)(Il2CppString*, const Il2CppMethod*))(base + OFFSET_SHADER_FIND);
    o_Material_ctor = (void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_MATERIAL_CTOR);
    o_Material_set_mainTexture = (void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_MATERIAL_SET_MAINTEXTURE);
    o_Renderer_get_material = (uintptr_t(__fastcall*)(uintptr_t))(base + OFFSET_RENDERER_GET_MATERIAL);
    o_Renderer_set_material = (void(__fastcall*)(uintptr_t, uintptr_t))(base + OFFSET_RENDERER_SET_MATERIAL);
    o_Material_get_color = (Color(__fastcall*)(uintptr_t))(base + OFFSET_MATERIAL_GET_COLOR);
    o_Material_set_color = (void(__fastcall*)(uintptr_t, Color))(base + OFFSET_MATERIAL_SET_COLOR);
    o_Material_set_renderQueue = (void(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_SET_RENDERQUEUE);
    o_RenderSettings_get_skybox = (uintptr_t(__fastcall*)(const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_SKYBOX);
    o_RenderSettings_set_skybox = (void(__fastcall*)(uintptr_t, const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_SET_SKYBOX);

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



    o_Component_get_gameObject = (uintptr_t(__fastcall*)(uintptr_t))(base + OFFSET_COMPONENT_GET_GAMEOBJECT);
    o_GameObject_get_activeInHierarchy = (bool(__fastcall*)(uintptr_t))(base + OFFSET_GAMEOBJECT_GET_ACTIVEINHIERARCHY);
    o_CanvasGroup_set_alpha = (void(__fastcall*)(uintptr_t, float))(base + OFFSET_CANVASGROUP_SET_ALPHA);
    MH_CreateHook((LPVOID)(base + OFFSET_GAMEOBJECT_SETACTIVE), hk_GameObject_SetActive, (LPVOID*)&o_GameObject_SetActive);
    MH_EnableHook((LPVOID)(base + OFFSET_GAMEOBJECT_SETACTIVE));
    MH_CreateHook((LPVOID)(base + OFFSET_AIMVIEW_AWAKE), hk_AimView_Awake, (LPVOID*)&o_AimView_Awake);
    MH_EnableHook((LPVOID)(base + OFFSET_AIMVIEW_AWAKE));
    MH_CreateHook((LPVOID)(base + OFFSET_AIMVIEW_UPDATE_SNIPER_PANELS), hk_AimView_UpdateSniperPanels, (LPVOID*)&o_AimView_UpdateSniperPanels);
    MH_EnableHook((LPVOID)(base + OFFSET_AIMVIEW_UPDATE_SNIPER_PANELS));
    MH_CreateHook((LPVOID)(base + OFFSET_HUDVIEW_UPDATE), hk_HUDView_Update, (LPVOID*)&o_HUDView_Update);
    MH_EnableHook((LPVOID)(base + OFFSET_HUDVIEW_UPDATE));

    // Fire is only a synchronous scope marker; HitCaster supplies the real spread-adjusted path.
    MH_CreateHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE), hk_GunController_Fire, (LPVOID*)&o_GunController_Fire);
    MH_EnableHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE));
    MH_CreateHook((LPVOID)(base + OFFSET_HITCASTER_CAST), hk_HitCaster_Cast, (LPVOID*)&o_HitCaster_Cast);
    MH_EnableHook((LPVOID)(base + OFFSET_HITCASTER_CAST));

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
