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

struct NativeRaycastHit {
    Vector3 point;       // +0x00
    Vector3 normal;      // +0x0C
    uint32_t faceId;     // +0x18
    float distance;      // +0x1C
    float uvX;           // +0x20
    float uvY;           // +0x24
    int collider;        // +0x28
};

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
#define OFFSET_TIME_GET_DELTATIME                   0x2F048A0

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
#define OFFSET_MATERIAL_COPY_CTOR                  0x2ECEB30
#define OFFSET_MATERIAL_SET_SHADER                 0x2ECF430
#define OFFSET_MATERIAL_SET_MAINTEXTURE            0x2ECF320
#define OFFSET_RENDERER_GET_MATERIAL                0x2ED8EA0
#define OFFSET_RENDERER_SET_MATERIAL                0x2ED9230
#define OFFSET_RENDERER_GET_MATERIALS                0x2ED8E20
#define OFFSET_RENDERER_IS_STATIC_BATCH              0x2ED9370
#define OFFSET_RENDERER_SET_MATERIALS                0x2ED91C0
#define OFFSET_OBJECT_FIND_OBJECTS_OF_TYPE            0x2EF9B30
#define OFFSET_MATERIAL_GET_COLOR                   0x2ECEBB0
#define OFFSET_MATERIAL_SET_COLOR                   0x2ECF040
#define OFFSET_MATERIAL_SET_RENDERQUEUE             0x2ECF3F0
#define OFFSET_MATERIAL_SET_FLOAT                   0x2ECE360
#define OFFSET_MATERIAL_HAS_PROPERTY                0x2ECE050
#define OFFSET_MATERIAL_GET_COLOR_ID                0x2ECD9E0
#define OFFSET_MATERIAL_SET_COLOR_ID                0x2ECE1E0
#define OFFSET_SHADER_PROPERTY_TO_ID                0x2ED9E40
#define OFFSET_RENDERSETTINGS_GET_SKYBOX            0x2ED8D60
#define OFFSET_RENDERSETTINGS_SET_SKYBOX            0x2ED8D90
#define OFFSET_GLOVES_SET_ARMS                    0x8F8110
#define OFFSET_ARMSLOD_SET_VISIBLE                 0x81F1E0
#define OFFSET_WEAPONRY_TAKE_WEAPON                0x8491C0
#define OFFSET_GUNCONTROLLER_FIRE                  0x996BE0
#define OFFSET_RAYCASTER_QMK                       0x8470C0
#define OFFSET_HITCASTER_CAST                      0x99FBB0
#define OFFSET_AIMVIEW_AWAKE                       0xA60AA0
#define OFFSET_AIMVIEW_UPDATE_SNIPER_PANELS         0xA61960
#define OFFSET_HUDVIEW_UPDATE                       0xA68C60
#define OFFSET_HITMARKERVIEW_SHOW                    0xA6AFC0
#define OFFSET_CHEAT_RUNTIME_SET_THIRDPERSON       0xA5D6C0
#define OFFSET_PLAYERCONTROLLER_COMMAND             0x83D210
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_A          0x840BF0  // PlayerManager.qkb(PlayerController)
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_B          0x840DE0  // PlayerManager.qkc(PlayerController)
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_C          0x840E10  // PlayerManager.qkd(PlayerController)
#define OFFSET_WEAPONCONTROLLER_GET_ID                0x5409B0  // WeaponController.whs() -> glg
#define OFFSET_PLAYER_HEALTH_GET_CURRENT               0x8B77E0  // bqw.scy() after network updates
#define OFFSET_PLAYER_HEALTH_APPLY_A                   0x8B6B70  // bqw.scu(...)
#define OFFSET_PLAYER_HEALTH_APPLY_B                   0x8B6F90  // bqw.scv(...)
#define OFFSET_PLAYER_HEALTH_APPLY_C                   0x8B7210  // bqw.scw(...)
#define OFFSET_PLAYER_HEALTH_APPLY_D                   0x8B7660  // bqw.scx(...)
#define OFFSET_PLAYER_HIT_CONFIRMED_A                  0x87A690  // PlayerHitController.ria(boo)
#define OFFSET_PLAYER_HIT_CONFIRMED_B                  0x87A890  // PlayerHitController.rib(boo)
#define OFFSET_DAMAGE_RESULT_GET_HEALTH                0x880990  // boo.rfu() / dcxl
#define OFFSET_KEYBOARDCONTROL_BUILD_COMMAND        0xA6E220
#define OFFSET_AIMCONTROLLER_GET_SNAPSHOT           0x86C0E0
#define OFFSET_AIMINGDATA_CLONE                      0x86F9C0
#define OFFSET_TRANSFORM_GET_EULERANGLES            0x2F066B0
#define OFFSET_TRANSFORM_SET_EULERANGLES            0x2F07020
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

#define OFFSET_RESERVE_AMMO                        0xE4  // short cegx / wyw()
#define OFFSET_CURRENT_AMMO                        0xE6  // short cegy / wyy()




// GunController methods (wyw() / wyy())

#define OFFSET_GUNCONTROLLER_GETCURRENTAMMO        0x999180  // wyy() -> cegy, current magazine




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
bool silentAntiAimEnabled = false;
bool silentAntiAimHookReady = false;
volatile LONG silentAntiAimInputBuildCalls = 0;
volatile LONG silentAntiAimCommandCalls = 0;
volatile LONG silentAntiAimAppliedCalls = 0;
bool silentAntiAimLatestInputValid = false;
bool silentAntiAimLatestFiring = false;
uintptr_t silentAntiAimLatestPlayer = 0;
Vector3 silentAntiAimLatestRealAimAngle;
Vector3 silentAntiAimLatestRealAimEuler;
volatile LONG silentAntiAimSnapshotCalls = 0;
char silentAntiAimStatus[96] = "Disabled";
bool thirdPersonEnabled = false;
uintptr_t customizedThirdPersonPlayer = 0;
Vector3 originalDefaultTpsOffset;
Vector3 originalCrouchingTpsOffset;
bool originalTpsOffsetsCaptured = false;
float thirdPersonHorizontalOffset = 0.0f;
float thirdPersonHeightAdjustment = 0.0f;
float thirdPersonDistanceAdjustment = 0.0f;
volatile LONG pendingThirdPersonCommand = 0;
Il2CppClass* g_GameControllerClass = nullptr;
Il2CppField* g_GameControllerInstanceField = nullptr;
void(__fastcall* o_CheatRuntime_SetThirdPerson)(uintptr_t, bool) = nullptr;
char thirdPersonStatus[96] = "Disabled";
bool worldColorEnabled = false;
float worldColor[3] = { 0.35f, 0.55f, 1.0f };
float worldColorStrength = 0.35f;
int worldColorMode = 0; // Textured Tint, Flat, Glass, Lit, Metallic, Rainbow, Pulse
float worldColorAlpha = 0.35f;
float worldColorMetallic = 0.9f;
float worldColorSmoothness = 0.8f;
float worldColorAnimationSpeed = 1.0f;
ULONGLONG worldColorLastAnimationTick = 0;
struct WorldColorRenderer { uintptr_t renderer; std::vector<uintptr_t> originalMaterials; std::vector<uintptr_t> tintedMaterials; };
struct WorldColorTintMaterial { uintptr_t material; Color originalColor; };
std::vector<WorldColorRenderer> worldColorRenderers;
std::vector<WorldColorTintMaterial> worldColorTintMaterials;
uintptr_t worldColorMaterials[7] = {};
uint32_t worldColorMaterialHandles[7] = {};
volatile LONG pendingWorldColorCommand = 0;
ULONGLONG worldColorNextRetryTick = 0;
int worldColorRetryCount = 0;
const int worldColorMaxRetries = 12;
char worldColorStatus[128] = "Disabled";
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
uintptr_t frozenAmmoWeapon = 0;
short frozenAmmoValue = -1;
volatile LONG infinityAmmoFireCalls = 0;
volatile LONG infinityAmmoGetterCalls = 0;
volatile LONG infinityAmmoRestores = 0;
volatile LONG infinityAmmoLastField = -1;
volatile LONG infinityAmmoLastGetter = -1;
bool aimbotEnabled = false;              // silent shot redirection (kept unchanged)
bool visibleAimbotEnabled = false;       // visibly turns the real camera, then restores it
bool aimbotVisibleCheck = true;
float aimbotFov = 360.0f;
float visibleAimbotHoldMs = 140.0f;
bool aimbotAutoFire = false;
uintptr_t aimbotLastHitParameters = 0;
volatile LONG aimbotAutoFired = 0;
ULONGLONG aimbotVisibilityNextScanAt = 0;
ULONGLONG aimbotVisibilityValidUntil = 0;
bool aimbotCachedVisibleTarget = false;
Vector3 aimbotCachedVisibleDirection;
volatile LONG aimbotVisibilityScans = 0;
bool visibleAimbotCameraActive = false;
uintptr_t visibleAimbotAimingData = 0;
Vector3 visibleAimbotOriginalAimAngle;
Vector3 visibleAimbotOriginalAimEuler;
Vector3 visibleAimbotTargetAim;
ULONGLONG visibleAimbotRestoreAt = 0;
volatile LONG aimbotShots = 0;
volatile LONG aimbotTargetsScanned = 0;
volatile LONG aimbotVisibleTargets = 0;
volatile LONG aimbotApplied = 0;
char aimbotStatus[96] = "Disabled";

bool hitMarkerEnabled = false;
float hitMarkerColor[3] = { 1.0f, 1.0f, 1.0f };
float hitMarkerDuration = 0.22f;
float hitMarkerSize = 9.0f;
float hitMarkerGap = 4.0f;
float hitMarkerThickness = 2.0f;
volatile LONG hitMarkerCalls = 0;
volatile LONG hitMarkerResolvedCalls = 0;
volatile LONG hitMarkerFallbackCalls = 0;
uintptr_t liveHitMarkerView = 0;
Vector3 latestHitMarkerCastStart;
Vector3 latestHitMarkerCastEnd;
bool latestHitMarkerCastValid = false;
ULONGLONG latestHitMarkerCastAt = 0;
struct HitMarkerEntry {
    Vector3 worldPosition;
    ULONGLONG triggeredAt;
    bool screenCenterFallback;
};
std::vector<HitMarkerEntry> hitMarkers;
SRWLOCK hitMarkerLock = SRWLOCK_INIT;
bool bulletTracerEnabled = false;
bool noSpreadEnabled = false;
bool removeScopeBorders = false;
bool customScopeReticleVisible = false;
uintptr_t liveAimView = 0;
uintptr_t liveHudLocalPlayer = 0;
uintptr_t sniperSightObject = 0;
volatile LONG pendingScopeOverlayRefresh = 0;
float bulletTracerStartColor[3] = { 1.0f, 0.15f, 0.05f };
float bulletTracerEndColor[3] = { 0.15f, 0.55f, 1.0f };
float bulletTracerDuration = 1.5f;
float bulletTracerThickness = 2.5f;

bool weaponChamsEnabled = false;
int weaponChamsMode = 0; // Flat, Glass, Lit, Metallic, Transparent Lit, Rainbow, Pulse
float weaponChamsColor[3] = { 0.05f, 0.75f, 1.0f };
float weaponChamsGlassAlpha = 0.35f;
float weaponChamsMetallic = 0.9f;
float weaponChamsSmoothness = 0.8f;
float weaponChamsAnimationSpeed = 1.0f;
ULONGLONG weaponChamsLastAnimationTick = 0;

struct WeaponChamsRenderer {
    uintptr_t renderer;
    uintptr_t originalMaterial;
};
std::vector<WeaponChamsRenderer> weaponChamsRenderers;
uintptr_t weaponChamsController = 0;
uintptr_t activeLocalWeaponController = 0;
uintptr_t chamsObservedLocalPlayer = 0;
uintptr_t weaponChamsMaterials[7] = {};
uint32_t weaponChamsMaterialHandles[7] = {};
char weaponChamsStatus[128] = "Select Flat or Glass";
volatile LONG pendingWeaponChamsRefresh = 0;

bool armChamsEnabled = false;
int armChamsMode = 0;
float armChamsColor[3] = { 1.0f, 0.25f, 0.65f };
float armChamsAlpha = 0.35f;
float armChamsMetallic = 0.9f;
float armChamsSmoothness = 0.8f;
float armChamsAnimationSpeed = 1.0f;
ULONGLONG armChamsLastAnimationTick = 0;
uintptr_t armChamsMaterials[7] = {};
uint32_t armChamsMaterialHandles[7] = {};
struct ArmChamsRenderer {
    uintptr_t renderer;
    uintptr_t originalMaterial;
};
std::vector<ArmChamsRenderer> armChamsRenderers;
uintptr_t armChamsArmsLodGroup = 0;
char armChamsStatus[128] = "Disabled";
volatile LONG pendingArmChamsRefresh = 0;

bool gloveChamsEnabled = false;
int gloveChamsMode = 0;
float gloveChamsColor[3] = { 0.2f, 0.85f, 1.0f };
float gloveChamsAlpha = 0.35f;
float gloveChamsMetallic = 0.9f;
float gloveChamsSmoothness = 0.8f;
float gloveChamsAnimationSpeed = 1.0f;
ULONGLONG gloveChamsLastAnimationTick = 0;
uintptr_t gloveChamsMaterials[7] = {};
uint32_t gloveChamsMaterialHandles[7] = {};
struct GloveChamsRenderer { uintptr_t renderer; uintptr_t originalMaterial; };
std::vector<GloveChamsRenderer> gloveChamsRenderers;
uintptr_t gloveChamsArmsLodGroup = 0;
char gloveChamsStatus[128] = "Disabled";
volatile LONG pendingGloveChamsRefresh = 0;
ULONGLONG armGloveChamsLastMaintenanceTick = 0;

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
float velocityLimiterCurrentHorizontalSpeed = 0.0f;
float velocityLimiterLastAppliedScale = 1.0f;



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
bool espShowName = true;
bool espShowHealth = true;
bool espShowWeapon = true;
bool espGradient = true;
bool espHealthGradient = true;
float espTopColor[3] = { 1.0f, 0.20f, 0.35f };
float espBottomColor[3] = { 0.55f, 0.10f, 1.0f };
float espNameColor[3] = { 1.0f, 0.25f, 0.55f };
float espHealthColor[3] = { 0.20f, 1.0f, 0.25f };
float espHealthBottomColor[3] = { 1.0f, 0.15f, 0.10f };
volatile LONG boxEspEnumerated = 0;
volatile LONG boxEspProjected = 0;
volatile LONG boxEspDrawn = 0;
volatile LONG boxEspDictionaryCount = 0;
volatile LONG boxEspUnityCount = 0;
volatile LONG boxEspHookCount = 0;

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
extern uintptr_t(__fastcall* o_GetPlayerController)();
extern Il2CppArray*(__fastcall* o_Object_FindObjectsOfType)(Il2CppObject*, bool, const Il2CppMethod*);

Il2CppClass* g_PlayerManagerClass = nullptr;
Il2CppField* g_PlayerManagerInstanceField = nullptr;
Il2CppObject* g_PlayerControllerReflectionType = nullptr;
std::vector<void*> g_LivePlayerRegistry;
SRWLOCK g_LivePlayerRegistryLock = SRWLOCK_INIT;

std::unordered_map<uintptr_t, int> g_LiveHealthByState;
std::unordered_map<uintptr_t, int> g_LiveHealthByPlayer;
std::unordered_map<uintptr_t, int> g_ConfirmedHealthByPlayer;
SRWLOCK g_LiveHealthLock = SRWLOCK_INIT;
volatile LONG boxEspHealthUpdates = 0;
volatile LONG boxEspHealthPlayerMatches = 0;
volatile LONG boxEspHealthLastA = -1;
volatile LONG boxEspHealthLastB = -1;
volatile LONG boxEspHealthLastRead = -1;
volatile LONG boxEspConfirmedHits = 0;
volatile LONG boxEspConfirmedHealth = -1;
volatile LONG boxEspConfirmedFieldA = -1;
volatile LONG boxEspConfirmedFieldB = -1;
volatile LONG boxEspConfirmedFieldC = -1;

typedef void(__fastcall* t_PlayerManagerPlayerEvent)(uintptr_t, void*, const Il2CppMethod*);
t_PlayerManagerPlayerEvent o_PlayerManagerPlayerEventA = nullptr;
t_PlayerManagerPlayerEvent o_PlayerManagerPlayerEventB = nullptr;
t_PlayerManagerPlayerEvent o_PlayerManagerPlayerEventC = nullptr;

static void ForgetCachedHealthForPlayer(void* player) {
    if (!player) return;
    AcquireSRWLockExclusive(&g_LiveHealthLock);
    g_LiveHealthByPlayer.erase(reinterpret_cast<uintptr_t>(player));
    g_ConfirmedHealthByPlayer.erase(reinterpret_cast<uintptr_t>(player));
    ReleaseSRWLockExclusive(&g_LiveHealthLock);
}

static void RememberLivePlayer(void* player) {
    if (!player) return;
    ForgetCachedHealthForPlayer(player);
    AcquireSRWLockExclusive(&g_LivePlayerRegistryLock);
    if (std::find(g_LivePlayerRegistry.begin(), g_LivePlayerRegistry.end(), player) == g_LivePlayerRegistry.end()) {
        g_LivePlayerRegistry.push_back(player);
        if (g_LivePlayerRegistry.size() > 128)
            g_LivePlayerRegistry.erase(g_LivePlayerRegistry.begin(), g_LivePlayerRegistry.begin() + (g_LivePlayerRegistry.size() - 128));
    }
    ReleaseSRWLockExclusive(&g_LivePlayerRegistryLock);
}

void __fastcall hk_PlayerManagerPlayerEventA(uintptr_t instance, void* player, const Il2CppMethod* method) {
    RememberLivePlayer(player);
    o_PlayerManagerPlayerEventA(instance, player, method);
}
void __fastcall hk_PlayerManagerPlayerEventB(uintptr_t instance, void* player, const Il2CppMethod* method) {
    RememberLivePlayer(player);
    o_PlayerManagerPlayerEventB(instance, player, method);
}
void __fastcall hk_PlayerManagerPlayerEventC(uintptr_t instance, void* player, const Il2CppMethod* method) {
    RememberLivePlayer(player);
    o_PlayerManagerPlayerEventC(instance, player, method);
}

using t_HealthApplyA = void(__fastcall*)(uintptr_t, int, int, bool, bool, float, const Il2CppMethod*);
using t_HealthApplyB = void(__fastcall*)(uintptr_t, int, int, bool, bool, float, void*, const Il2CppMethod*);
using t_HealthApplyC = void(__fastcall*)(uintptr_t, int, bool, const Il2CppMethod*);
t_HealthApplyA o_HealthApplyA = nullptr;
t_HealthApplyB o_HealthApplyB = nullptr;
t_HealthApplyC o_HealthApplyC = nullptr;
t_HealthApplyC o_HealthApplyD = nullptr;

static int ReadHealthStateNow(uintptr_t state) {
    if (!state || !base) return -1;
    __try {
        using GetHealthFn = int(__fastcall*)(uintptr_t, const Il2CppMethod*);
        static GetHealthFn getHealth = nullptr;
        if (!getHealth) getHealth = reinterpret_cast<GetHealthFn>(base + OFFSET_PLAYER_HEALTH_GET_CURRENT);
        const int hp = getHealth(state, nullptr);
        return hp >= 0 && hp <= 100 ? hp : -1;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
}
static void CacheUpdatedHealth(uintptr_t state) {
    const int hp = ReadHealthStateNow(state);
    if (hp < 0) return;
    uintptr_t player = 0;
    __try { player = *reinterpret_cast<uintptr_t*>(state + 0x40); } // bqw.cbcu
    __except (EXCEPTION_EXECUTE_HANDLER) { player = 0; }
    AcquireSRWLockExclusive(&g_LiveHealthLock);
    g_LiveHealthByState[state] = hp;
    if (player) g_LiveHealthByPlayer[player] = hp;
    if (g_LiveHealthByState.size() > 256) g_LiveHealthByState.clear();
    if (g_LiveHealthByPlayer.size() > 256) g_LiveHealthByPlayer.clear();
    ReleaseSRWLockExclusive(&g_LiveHealthLock);
    InterlockedExchange(&boxEspHealthLastRead, hp);
    InterlockedIncrement(&boxEspHealthUpdates);
}
void __fastcall hk_HealthApplyA(uintptr_t state, int a, int b, bool c, bool d, float e, const Il2CppMethod* method) {
    o_HealthApplyA(state, a, b, c, d, e, method);
    InterlockedExchange(&boxEspHealthLastA, a); InterlockedExchange(&boxEspHealthLastB, b); CacheUpdatedHealth(state);
}
void __fastcall hk_HealthApplyB(uintptr_t state, int a, int b, bool c, bool d, float e, void* info, const Il2CppMethod* method) {
    o_HealthApplyB(state, a, b, c, d, e, info, method);
    InterlockedExchange(&boxEspHealthLastA, a); InterlockedExchange(&boxEspHealthLastB, b); CacheUpdatedHealth(state);
}
void __fastcall hk_HealthApplyC(uintptr_t state, int a, bool b, const Il2CppMethod* method) {
    o_HealthApplyC(state, a, b, method);
    InterlockedExchange(&boxEspHealthLastA, a); InterlockedExchange(&boxEspHealthLastB, -1); CacheUpdatedHealth(state);
}
void __fastcall hk_HealthApplyD(uintptr_t state, int a, bool b, const Il2CppMethod* method) {
    o_HealthApplyD(state, a, b, method);
    InterlockedExchange(&boxEspHealthLastA, a); InterlockedExchange(&boxEspHealthLastB, -1); CacheUpdatedHealth(state);
}

using t_PlayerHitConfirmed = void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*);
t_PlayerHitConfirmed o_PlayerHitConfirmedA = nullptr;
t_PlayerHitConfirmed o_PlayerHitConfirmedB = nullptr;

static void CacheConfirmedDamageResult(uintptr_t hitController, uintptr_t result) {
    if (!hitController || !result || !base) return;
    int fieldA = -1, fieldB = -1, fieldC = -1, hp = -1;
    uintptr_t player = 0;
    __try {
        player = *reinterpret_cast<uintptr_t*>(hitController + 0x90); // HitController.cara
        fieldA = *reinterpret_cast<int*>(result + 0x18);              // boq.carn
        fieldB = *reinterpret_cast<int*>(result + 0x1C);              // boq.caro
        fieldC = *reinterpret_cast<int*>(result + 0x20);              // boq.carp
        using GetResultHealthFn = int(__fastcall*)(uintptr_t, const Il2CppMethod*);
        static GetResultHealthFn getResultHealth = nullptr;
        if (!getResultHealth) getResultHealth = reinterpret_cast<GetResultHealthFn>(base + OFFSET_DAMAGE_RESULT_GET_HEALTH);
        hp = getResultHealth(result, nullptr); // the same value KillerDetailsView uses for its health bar
    } __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    InterlockedExchange(&boxEspConfirmedFieldA, fieldA);
    InterlockedExchange(&boxEspConfirmedFieldB, fieldB);
    InterlockedExchange(&boxEspConfirmedFieldC, fieldC);
    InterlockedExchange(&boxEspConfirmedHealth, hp);
    InterlockedIncrement(&boxEspConfirmedHits);
    if (!player || hp < 0 || hp > 100) return;
    AcquireSRWLockExclusive(&g_LiveHealthLock);
    g_ConfirmedHealthByPlayer[player] = hp;
    ReleaseSRWLockExclusive(&g_LiveHealthLock);
}
void __fastcall hk_PlayerHitConfirmedA(uintptr_t hitController, uintptr_t result, const Il2CppMethod* method) {
    o_PlayerHitConfirmedA(hitController, result, method);
    CacheConfirmedDamageResult(hitController, result);
}
void __fastcall hk_PlayerHitConfirmedB(uintptr_t hitController, uintptr_t result, const Il2CppMethod* method) {
    o_PlayerHitConfirmedB(hitController, result, method);
    CacheConfirmedDamageResult(hitController, result);
}
Il2CppClass* g_GlovesManagerClass = nullptr;
Il2CppField* g_GlovesManagerInstanceField = nullptr;

static void* GetPlayerManagerInstance() {
    if (!g_il2cpp.field_static_get_value || !g_PlayerManagerInstanceField) return nullptr;
    void* inst = nullptr;
    g_il2cpp.field_static_get_value(g_PlayerManagerInstanceField, &inst);
    return inst;
}

static void* GetGlovesManagerInstance() {
    if (!g_il2cpp.field_static_get_value || !g_GlovesManagerInstanceField) return nullptr;
    void* inst = nullptr;
    g_il2cpp.field_static_get_value(g_GlovesManagerInstanceField, &inst);
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
static void CollectPlayersFromDictionary(uintptr_t dict, void** out, int maxN, int& outN) {
    if (!dict || !out || maxN <= 0) return;
    __try {
        const uintptr_t entries = *reinterpret_cast<uintptr_t*>(dict + 0x18);
        const int count = *reinterpret_cast<int*>(dict + 0x20);
        if (!entries || count <= 0 || count > 256) return;
        const int entrySize = 0x18;
        for (int i = 0; i < count && outN < maxN; ++i) {
            const uintptr_t entry = entries + 0x20 + static_cast<uintptr_t>(i) * entrySize;
            if (*reinterpret_cast<int*>(entry + 0x00) < 0) continue;
            void* player = *reinterpret_cast<void**>(entry + 0x10);
            if (!player) continue;
            bool duplicate = false;
            for (int existing = 0; existing < outN; ++existing) {
                if (out[existing] == player) { duplicate = true; break; }
            }
            if (!duplicate) out[outN++] = player;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void CollectPlayers(void** out, int maxN, int& outN) {
    outN = 0;
    int dictionaryCount = 0;
    int unityCount = 0;
    void* pm = GetPlayerManagerInstance();
    __try {
        if (pm) {
            CollectPlayersFromDictionary(*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pm) + 0x28), out, maxN, outN);
            if (outN <= 1)
                CollectPlayersFromDictionary(*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pm) + 0x50), out, maxN, outN);
            dictionaryCount = outN;
        }
        if (outN <= 1 && g_PlayerControllerReflectionType && o_Object_FindObjectsOfType) {
            Il2CppArray* objects = o_Object_FindObjectsOfType(g_PlayerControllerReflectionType, true, nullptr);
            const uintptr_t array = reinterpret_cast<uintptr_t>(objects);
            if (array) {
                const uintptr_t length = *reinterpret_cast<uintptr_t*>(array + 0x18);
                if (length > 0 && length <= 256) {
                    for (uintptr_t i = 0; i < length && outN < maxN; ++i) {
                        void* player = *reinterpret_cast<void**>(array + 0x20 + i * sizeof(uintptr_t));
                        if (!player) continue;
                        bool duplicate = false;
                        for (int existing = 0; existing < outN; ++existing)
                            if (out[existing] == player) { duplicate = true; break; }
                        if (!duplicate) out[outN++] = player;
                    }
                }
            }
            unityCount = outN - dictionaryCount;
        }
        // Definitive source: PlayerManager's own live player callbacks. This bypasses
        // all generic Dictionary/FindObjectsOfType layout and overload differences.
        AcquireSRWLockShared(&g_LivePlayerRegistryLock);
        const int hookRegistrySize = static_cast<int>(g_LivePlayerRegistry.size());
        for (void* player : g_LivePlayerRegistry) {
            if (outN >= maxN) break;
            bool duplicate = false;
            for (int existing = 0; existing < outN; ++existing)
                if (out[existing] == player) { duplicate = true; break; }
            if (!duplicate) out[outN++] = player;
        }
        ReleaseSRWLockShared(&g_LivePlayerRegistryLock);
        InterlockedExchange(&boxEspDictionaryCount, dictionaryCount);
        InterlockedExchange(&boxEspUnityCount, unityCount);
        InterlockedExchange(&boxEspHookCount, hookRegistrySize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        outN = 0;
        InterlockedExchange(&boxEspDictionaryCount, 0);
        InterlockedExchange(&boxEspUnityCount, 0);
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

static unsigned char GetPCTeam(void* pc) {
    if (!pc) return 0;
    __try {
        // cwf: None=0, Tr=1, Ct=2, Spectator=3.
        return *reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(pc) + PC_TEAM_ENUM);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static bool Il2CppStringToUtf8(void* stringObject, char* output, int outputSize) {
    if (!output || outputSize <= 1) return false;
    output[0] = '\0';
    if (!stringObject) return false;
    __try {
        const int length = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(stringObject) + 0x10);
        if (length <= 0 || length > 128) return false;
        const wchar_t* chars = reinterpret_cast<const wchar_t*>(reinterpret_cast<uintptr_t>(stringObject) + 0x14);
        const int bytes = WideCharToMultiByte(CP_UTF8, 0, chars, length, output, outputSize - 1, nullptr, nullptr);
        if (bytes <= 0 || bytes >= outputSize) { output[0] = '\0'; return false; }
        output[bytes] = '\0';
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { output[0] = '\0'; return false; }
}

static void GetPCName(void* pc, char* output, int outputSize) {
    if (!output || outputSize <= 1) return;
    strcpy_s(output, outputSize, "Enemy");
    if (!pc) return;
    __try {
        void* primary = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pc) + 0x158);
        if (Il2CppStringToUtf8(primary, output, outputSize)) return;
        void* fallback = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pc) + 0x160);
        if (!Il2CppStringToUtf8(fallback, output, outputSize))
            strcpy_s(output, outputSize, "Enemy");
    } __except (EXCEPTION_EXECUTE_HANDLER) { strcpy_s(output, outputSize, "Enemy"); }
}

static int GetPCHealth(void* pc) {
    if (!pc) return -1;
    uintptr_t states[3] = {};
    __try {
        states[0] = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pc) + 0x148);
        const uintptr_t network = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pc) + 0x108);
        if (network) states[1] = *reinterpret_cast<uintptr_t*>(network + 0xA0);
        const uintptr_t hit = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pc) + 0xF0);
        if (hit) states[2] = *reinterpret_cast<uintptr_t*>(hit + 0x50);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
    // Deliberately no cross-frame cache here: a hashmap keyed by pointer breaks the moment
    // Unity/Il2Cpp reuses a PlayerController/bqw address for a respawned or newly joined
    // player, silently showing that address's last (possibly dead) value on a full-health
    // player who was never hit. The state pointer is recomputed from pc every call, so a
    // respawn/rejoin naturally resolves to a brand-new bqw with its own fresh value.
    int bestHp = 101;
    for (uintptr_t state : states) { const int hp = ReadHealthStateNow(state); if (hp >= 0 && hp < bestHp) bestHp = hp; }
    if (bestHp <= 100) { InterlockedExchange(&boxEspHealthLastRead, bestHp); return bestHp; }
    AcquireSRWLockShared(&g_LiveHealthLock);
    const auto confirmedIt = g_ConfirmedHealthByPlayer.find(reinterpret_cast<uintptr_t>(pc));
    const bool hasConfirmed = confirmedIt != g_ConfirmedHealthByPlayer.end();
    const int confirmedHp = hasConfirmed ? confirmedIt->second : -1;
    ReleaseSRWLockShared(&g_LiveHealthLock);
    return hasConfirmed ? confirmedHp : -1;
}

static const char* WeaponNameFromId(unsigned char id) {
    switch (id) {
    case 11: return "G22"; case 12: return "USP"; case 13: return "P350"; case 15: return "Deagle";
    case 16: return "Tec-9"; case 17: return "Five-Seven"; case 18: return "Berettas";
    case 32: return "UMP45"; case 33: return "Akimbo Uzi"; case 34: return "MP7"; case 35: return "P90";
    case 36: return "MP5"; case 37: return "MAC10"; case 42: return "VAL"; case 43: return "M4A1";
    case 44: return "AKR"; case 45: return "AKR12"; case 46: return "M4"; case 47: return "M16";
    case 48: return "FAMAS"; case 49: return "FN FAL"; case 51: return "AWM"; case 52: return "M40";
    case 53: return "M110"; case 54: return "Mallard"; case 62: return "SM1014"; case 63: return "FabM";
    case 64: return "M60"; case 65: return "SPAS"; case 70: return "Knife"; case 89: return "Hands";
    case 91: return "HE"; case 92: return "Smoke"; case 93: return "Flash"; case 94: return "Molotov";
    case 95: return "Incendiary"; case 100: return "Bomb"; default: return id ? "Weapon" : "Unarmed";
    }
}

static const char* GetPCWeaponName(void* pc) {
    if (!pc) return "Unarmed";
    __try {
        const uintptr_t weaponry = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pc) + 0xD0);
        const uintptr_t weapon = weaponry ? *reinterpret_cast<uintptr_t*>(weaponry + 0xA0) : 0;
        if (!weapon) return "Unarmed";
        using GetWeaponIdFn = unsigned char(__fastcall*)(uintptr_t, const Il2CppMethod*);
        static GetWeaponIdFn getWeaponId = nullptr;
        if (!getWeaponId && base) getWeaponId = reinterpret_cast<GetWeaponIdFn>(base + OFFSET_WEAPONCONTROLLER_GET_ID);
        return getWeaponId ? WeaponNameFromId(getWeaponId(weapon, nullptr)) : "Weapon";
    } __except (EXCEPTION_EXECUTE_HANDLER) { return "Weapon"; }
}

static ImU32 EspArrayColor(const float color[3], float alpha = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], alpha));
}

static ImU32 EspLerpColor(const float top[3], const float bottom[3], float t, float alpha = 1.0f) {
    t = fmaxf(0.0f, fminf(1.0f, t));
    return ImGui::ColorConvertFloat4ToU32(ImVec4(
        top[0] + (bottom[0] - top[0]) * t,
        top[1] + (bottom[1] - top[1]) * t,
        top[2] + (bottom[2] - top[2]) * t, alpha));
}

static ImU32 EspHealthColorAt(float t) {
    return espHealthGradient ? EspLerpColor(espHealthColor, espHealthBottomColor, t) : EspArrayColor(espHealthColor);
}

static void DrawAsciiGradientText(ImDrawList* draw, ImVec2 pos, const char* text) {
    if (!draw || !text) return;
    const size_t length = strlen(text);
    float x = pos.x;
    for (size_t i = 0; i < length; ++i) {
        char glyph[2] = { text[i], '\0' };
        const float t = length > 1 ? static_cast<float>(i) / static_cast<float>(length - 1) : 0.0f;
        draw->AddText(ImVec2(x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), glyph);
        draw->AddText(ImVec2(x, pos.y), EspHealthColorAt(t), glyph);
        x += ImGui::CalcTextSize(glyph).x;
    }
}

static ImU32 EspColorAt(float t, float alpha = 1.0f) {
    t = fmaxf(0.0f, fminf(1.0f, t));
    const float r = espTopColor[0] + (espBottomColor[0] - espTopColor[0]) * t;
    const float g = espTopColor[1] + (espBottomColor[1] - espTopColor[1]) * t;
    const float b = espTopColor[2] + (espBottomColor[2] - espTopColor[2]) * t;
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, alpha));
}

static void* GetLocalPC() {
    void* pm = GetPlayerManagerInstance();
    __try {
        void* local = pm ? *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pm) + PM_LOCAL_PLAYER_BF) : nullptr;
        if (!local && o_GetPlayerController) local = reinterpret_cast<void*>(o_GetPlayerController());
        RememberLivePlayer(local);
        return local;
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
float(__fastcall* o_Time_get_deltaTime)() = nullptr;

Vector3(__fastcall* o_Transform_get_position)(uintptr_t) = nullptr;
Vector3(__fastcall* o_Transform_get_forward)(uintptr_t) = nullptr;
Vector3(__fastcall* o_Transform_get_eulerAngles)(uintptr_t) = nullptr;
void(__fastcall* o_Transform_set_eulerAngles)(uintptr_t, Vector3) = nullptr;

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
void(__fastcall* o_Material_copy_ctor)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Material_set_shader)(uintptr_t, uintptr_t) = nullptr;
void(__fastcall* o_Material_set_mainTexture)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Renderer_get_material)(uintptr_t) = nullptr;
void(__fastcall* o_Renderer_set_material)(uintptr_t, uintptr_t) = nullptr;
Il2CppArray*(__fastcall* o_Renderer_get_materials)(uintptr_t) = nullptr;
bool(__fastcall* o_Renderer_get_isPartOfStaticBatch)(uintptr_t) = nullptr;
void(__fastcall* o_Renderer_set_materials)(uintptr_t, Il2CppArray*) = nullptr;
Il2CppArray*(__fastcall* o_Object_FindObjectsOfType)(Il2CppObject*, bool, const Il2CppMethod*) = nullptr;
Color(__fastcall* o_Material_get_color)(uintptr_t) = nullptr;
void(__fastcall* o_Material_set_color)(uintptr_t, Color) = nullptr;
void(__fastcall* o_Material_set_renderQueue)(uintptr_t, int) = nullptr;
void(__fastcall* o_Material_SetFloat)(uintptr_t, Il2CppString*, float) = nullptr;
bool(__fastcall* o_Material_HasProperty)(uintptr_t, int) = nullptr;
Color(__fastcall* o_Material_GetColorId)(uintptr_t, int) = nullptr;
void(__fastcall* o_Material_SetColorId)(uintptr_t, int, Color) = nullptr;
int(__fastcall* o_Shader_PropertyToID)(Il2CppString*, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_RenderSettings_get_skybox)(const Il2CppMethod*) = nullptr;
void(__fastcall* o_RenderSettings_set_skybox)(uintptr_t, const Il2CppMethod*) = nullptr;

Vector3(__fastcall* o_WorldToScreenPoint)(uintptr_t, Vector3) = nullptr;

Matrix16(__fastcall* o_Camera_get_worldToCameraMatrix)(uintptr_t) = nullptr;

Matrix16(__fastcall* o_Camera_get_projectionMatrix)(uintptr_t) = nullptr;

short(__fastcall* o_GunController_GetCurrentAmmo)(uintptr_t) = nullptr;
void(__fastcall* o_Gloves_SetArms)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_ArmsLod_SetVisible)(uintptr_t, bool, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Weaponry_TakeWeapon)(uintptr_t, uint8_t, const Il2CppMethod*) = nullptr;
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
void(__fastcall* o_HitMarkerView_Show)(uintptr_t, bool, bool, const Il2CppMethod*) = nullptr;
void(__fastcall* o_PlayerController_Command)(uintptr_t, uintptr_t, float, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_KeyboardControl_BuildCommand)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_AimController_GetSnapshot)(uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_AimingData_Clone)(uintptr_t, const Il2CppMethod*) = nullptr;



static void SetRendererMaterialArray(uintptr_t renderer, const std::vector<uintptr_t>& materials)
{
    if (!renderer || materials.empty() || !o_Renderer_set_materials || !g_il2cpp.array_new) return;
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) return;
    Il2CppArray* array = g_il2cpp.array_new(materialClass, materials.size());
    const uintptr_t arrayAddress = reinterpret_cast<uintptr_t>(array);
    if (!arrayAddress) return;
    for (size_t i = 0; i < materials.size(); ++i)
        *reinterpret_cast<uintptr_t*>(arrayAddress + 0x20 + i * sizeof(uintptr_t)) = materials[i];
    o_Renderer_set_materials(renderer, array);
}

static uintptr_t GetCurrentLocalWeaponController();

static void RestoreWorldColorUnsafe()
{
    for (const WorldColorRenderer& entry : worldColorRenderers)
        SetRendererMaterialArray(entry.renderer, entry.originalMaterials);
    worldColorRenderers.clear();
    worldColorTintMaterials.clear();
    strcpy_s(worldColorStatus, "Disabled; original map materials restored");
}

static void RestoreWorldColor()
{
    __try { RestoreWorldColorUnsafe(); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        worldColorRenderers.clear();
        strcpy_s(worldColorStatus, "Restore stopped on a destroyed renderer");
    }
}

static uintptr_t EnsureWorldModeMaterial(int mode)
{
    if (mode < 1 || mode > 6) return 0;
    if (worldColorMaterials[mode]) return worldColorMaterials[mode];
    if (!o_Shader_Find || !o_Material_ctor || !g_il2cpp.object_new || !g_il2cpp.string_new) return 0;
    static const char* shaderCandidates[7][5] = {
        { nullptr, nullptr, nullptr, nullptr, nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Unlit/Transparent", "Legacy Shaders/Transparent/VertexLit", "Sprites/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr }
    };
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) return 0;
    uintptr_t shader = 0;
    for (const char* name : shaderCandidates[mode]) {
        if (!name) break;
        shader = o_Shader_Find(g_il2cpp.string_new(name), nullptr);
        if (shader) break;
    }
    if (!shader) return 0;
    const uintptr_t material = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(materialClass));
    if (!material) return 0;
    o_Material_ctor(material, shader, nullptr);
    if (mode == 2 && o_Material_set_renderQueue) o_Material_set_renderQueue(material, 3000);
    if (g_il2cpp.gchandle_new)
        worldColorMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    worldColorMaterials[mode] = material;
    return material;
}

static Color GetWorldTintColor(const Color& original)
{
    const float strength = (std::max)(0.0f, (std::min)(worldColorStrength, 1.0f));
    return Color(
        original.r * ((1.0f - strength) + worldColor[0] * strength),
        original.g * ((1.0f - strength) + worldColor[1] * strength),
        original.b * ((1.0f - strength) + worldColor[2] * strength), original.a);
}

static Color GetWorldAnimatedColor()
{
    if (worldColorMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * worldColorAnimationSpeed;
        return Color(0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.0943951f),
            0.5f + 0.5f * sinf(t + 4.1887902f), 1.0f);
    }
    if (worldColorMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * worldColorAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(worldColor[0] * intensity, worldColor[1] * intensity, worldColor[2] * intensity, 1.0f);
    }
    return Color(worldColor[0], worldColor[1], worldColor[2], worldColorMode == 2 ? worldColorAlpha : 1.0f);
}

static void ApplyWorldColorToCache()
{
    if (worldColorMode == 0) {
        if (!o_Material_set_color) return;
        for (const WorldColorTintMaterial& entry : worldColorTintMaterials)
            if (entry.material) o_Material_set_color(entry.material, GetWorldTintColor(entry.originalColor));
        for (const WorldColorRenderer& entry : worldColorRenderers)
            SetRendererMaterialArray(entry.renderer, entry.tintedMaterials);
        sprintf_s(worldColorStatus, "Textured tint: %zu map renderer(s)", worldColorRenderers.size());
        return;
    }

    const uintptr_t replacement = EnsureWorldModeMaterial(worldColorMode);
    if (!replacement || !o_Material_set_color) { strcpy_s(worldColorStatus, "Selected world shader unavailable"); return; }
    if (worldColorMode == 4 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), worldColorMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), worldColorSmoothness);
    }
    o_Material_set_color(replacement, GetWorldAnimatedColor());
    for (const WorldColorRenderer& entry : worldColorRenderers) {
        std::vector<uintptr_t> replacements(entry.originalMaterials.size(), replacement);
        SetRendererMaterialArray(entry.renderer, replacements);
    }
    static const char* names[] = { "Textured Tint", "Flat", "Glass", "Lit", "Metallic", "Rainbow", "Pulse" };
    sprintf_s(worldColorStatus, "%s: %zu map renderer(s)", names[worldColorMode], worldColorRenderers.size());
}

static void AnimateWorldColor()
{
    if (!worldColorEnabled || (worldColorMode != 5 && worldColorMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - worldColorLastAnimationTick < 33) return;
    worldColorLastAnimationTick = now;
    const uintptr_t material = worldColorMaterials[worldColorMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetWorldAnimatedColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void CaptureWorldColorMaterialsUnsafe()
{
    RestoreWorldColorUnsafe();
    const uintptr_t flatReplacement = EnsureWorldModeMaterial(1);
    if (!flatReplacement) { strcpy_s(worldColorStatus, "Could not create world material"); return; }
    uintptr_t texturedShader = o_Shader_Find(g_il2cpp.string_new("Legacy Shaders/Diffuse"), nullptr);
    if (!texturedShader) texturedShader = o_Shader_Find(g_il2cpp.string_new("Standard"), nullptr);
    if (!texturedShader) { strcpy_s(worldColorStatus, "Textured tint shader unavailable"); return; }
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) { strcpy_s(worldColorStatus, "Material class unavailable"); return; }
    Il2CppClass* meshRendererClass = g_il2cpp.find_class("UnityEngine", "MeshRenderer");
    if (!meshRendererClass) { strcpy_s(worldColorStatus, "MeshRenderer class unavailable"); return; }
    const Il2CppType* meshType = g_il2cpp.class_get_type(meshRendererClass);
    Il2CppObject* reflectionType = meshType ? g_il2cpp.type_get_object(meshType) : nullptr;
    Il2CppArray* renderers = reflectionType ? o_Object_FindObjectsOfType(reflectionType, false, nullptr) : nullptr;
    const uintptr_t rendererArray = reinterpret_cast<uintptr_t>(renderers);
    const size_t rendererCount = rendererArray ? *reinterpret_cast<size_t*>(rendererArray + 0x18) : 0;
    if (!rendererCount || rendererCount > 20000) { strcpy_s(worldColorStatus, "No active map renderers found"); return; }

    std::unordered_map<uintptr_t, bool> excluded;
    std::unordered_map<uintptr_t, uintptr_t> tintedByOriginal;
    worldColorTintMaterials.clear();
    for (const WeaponChamsRenderer& entry : weaponChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (const ArmChamsRenderer& entry : armChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (const GloveChamsRenderer& entry : gloveChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    const uintptr_t localWeapon = GetCurrentLocalWeaponController();
    if (localWeapon) {
        const uintptr_t lodGroup = *reinterpret_cast<uintptr_t*>(localWeapon + 0x80);
        const uintptr_t localRenderers = lodGroup ? *reinterpret_cast<uintptr_t*>(lodGroup + 0x40) : 0;
        const size_t localCount = localRenderers ? *reinterpret_cast<size_t*>(localRenderers + 0x18) : 0;
        if (localCount <= 64) for (size_t i = 0; i < localCount; ++i) {
            const uintptr_t renderer = *reinterpret_cast<uintptr_t*>(localRenderers + 0x20 + i * sizeof(uintptr_t));
            if (renderer) excluded[renderer] = true;
        }
    }

    for (size_t i = 0; i < rendererCount; ++i) {
        const uintptr_t renderer = *reinterpret_cast<uintptr_t*>(rendererArray + 0x20 + i * sizeof(uintptr_t));
        if (!renderer || excluded.find(renderer) != excluded.end()) continue;
        Il2CppArray* materials = o_Renderer_get_materials(renderer);
        const uintptr_t materialArray = reinterpret_cast<uintptr_t>(materials);
        const size_t materialCount = materialArray ? *reinterpret_cast<size_t*>(materialArray + 0x18) : 0;
        if (!materialCount || materialCount > 64) continue;
        WorldColorRenderer entry{};
        entry.renderer = renderer;
        entry.originalMaterials.reserve(materialCount);
        for (size_t j = 0; j < materialCount; ++j) {
            const uintptr_t material = *reinterpret_cast<uintptr_t*>(materialArray + 0x20 + j * sizeof(uintptr_t));
            entry.originalMaterials.push_back(material);
            uintptr_t tinted = 0;
            const auto found = tintedByOriginal.find(material);
            if (found != tintedByOriginal.end()) tinted = found->second;
            else if (material) {
                tinted = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(materialClass));
                if (tinted) {
                    o_Material_copy_ctor(tinted, material, nullptr);
                    o_Material_set_shader(tinted, texturedShader);
                    const Color originalColor = o_Material_get_color(tinted);
                    worldColorTintMaterials.push_back({ tinted, originalColor });
                    tintedByOriginal[material] = tinted;
                }
            }
            entry.tintedMaterials.push_back(tinted ? tinted : material);
        }
        worldColorRenderers.push_back(std::move(entry));
    }
    ApplyWorldColorToCache();
}

static bool CaptureWorldColorMaterials()
{
    if (!g_il2cpp.class_get_type || !g_il2cpp.type_get_object || !o_Object_FindObjectsOfType ||
        !o_Renderer_get_materials || !o_Renderer_set_materials || !o_Material_set_color ||
        !o_Material_get_color || !o_Material_copy_ctor || !o_Material_set_shader || !g_il2cpp.array_new) {
        strcpy_s(worldColorStatus, "World Color API unavailable");
        return false;
    }
    __try { CaptureWorldColorMaterialsUnsafe(); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        worldColorRenderers.clear();
        worldColorTintMaterials.clear();
        strcpy_s(worldColorStatus, "Map loading; retry scheduled");
        return false;
    }
    return !worldColorRenderers.empty();
}

static uintptr_t GetActiveGameController()
{
    if (!g_il2cpp.field_static_get_value || !g_GameControllerInstanceField) return 0;
    Il2CppObject* controller = nullptr;
    g_il2cpp.field_static_get_value(g_GameControllerInstanceField, &controller);
    return reinterpret_cast<uintptr_t>(controller);
}

static uintptr_t GetNativeCheatRuntime()
{
    const uintptr_t controller = GetActiveGameController();
    if (!controller || !g_il2cpp.class_get_name) return 0;
    __try {
        Il2CppClass* klass = *reinterpret_cast<Il2CppClass**>(controller);
        const char* name = klass ? g_il2cpp.class_get_name(klass) : nullptr;
        if (!name) return 0;
        struct RuntimeField { const char* className; uintptr_t offset; };
        static const RuntimeField fields[] = {
            { "OfflineModeController", 0x290 },
            { "GrenadeBattleWithBotsController", 0x2D8 },
            { "FreeForAllWithBotsController", 0x2B0 },
            { "DefuseWithBotsController", 0x2F0 },
            { "EscalationWithBotsController", 0x348 },
            { "DeathmatchWithBotsController", 0x2B8 },
            { "ArmsRaceWithBotsController", 0x2E0 },
            { "DuelV2WithBotsController", 0x290 },
            { "RankedDefuseController", 0x320 }
        };
        for (const RuntimeField& field : fields)
            if (strcmp(name, field.className) == 0)
                return *reinterpret_cast<uintptr_t*>(controller + field.offset);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 0;
}

static bool ApplyCustomizedThirdPersonOffsets()
{
    const uintptr_t player = liveHudLocalPlayer;
    if (!player) {
        if (thirdPersonEnabled) strcpy_s(thirdPersonStatus, "Waiting for local player camera offsets");
        return !thirdPersonEnabled;
    }
    __try {
        if (customizedThirdPersonPlayer != player) {
            customizedThirdPersonPlayer = player;
            originalTpsOffsetsCaptured = false;
        }
        if (!originalTpsOffsetsCaptured) {
            // PlayerController._defaultTpsOffset (+0x48) and _crouchingTpsOffset
            // (+0x54) are complete native Vector3 camera offsets.
            originalDefaultTpsOffset = *reinterpret_cast<Vector3*>(player + 0x48);
            originalCrouchingTpsOffset = *reinterpret_cast<Vector3*>(player + 0x54);
            originalTpsOffsetsCaptured = true;
        }
        if (thirdPersonEnabled) {
            Vector3 standing = originalDefaultTpsOffset;
            Vector3 crouching = originalCrouchingTpsOffset;
            // X is absolute so zero always means centered. Y/Z are adjustments
            // from the game's tuned standing and crouching camera positions.
            standing.x = thirdPersonHorizontalOffset;
            crouching.x = thirdPersonHorizontalOffset;
            standing.y += thirdPersonHeightAdjustment;
            crouching.y += thirdPersonHeightAdjustment;
            standing.z += thirdPersonDistanceAdjustment;
            crouching.z += thirdPersonDistanceAdjustment;
            *reinterpret_cast<Vector3*>(player + 0x48) = standing;
            *reinterpret_cast<Vector3*>(player + 0x54) = crouching;
        }
        else {
            *reinterpret_cast<Vector3*>(player + 0x48) = originalDefaultTpsOffset;
            *reinterpret_cast<Vector3*>(player + 0x54) = originalCrouchingTpsOffset;
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        originalTpsOffsetsCaptured = false;
        if (thirdPersonEnabled) strcpy_s(thirdPersonStatus, "Waiting for local player camera offsets");
        return !thirdPersonEnabled;
    }
}

static bool ApplyNativeThirdPersonState()
{
    if (!o_CheatRuntime_SetThirdPerson) {
        strcpy_s(thirdPersonStatus, "Native TPS handler unavailable");
        return false;
    }
    if (!ApplyCustomizedThirdPersonOffsets()) return false;
    const uintptr_t runtime = GetNativeCheatRuntime();
    if (!runtime) {
        strcpy_s(thirdPersonStatus, "Waiting for native cheat runtime");
        return false;
    }
    __try {
        o_CheatRuntime_SetThirdPerson(runtime, thirdPersonEnabled);
        strcpy_s(thirdPersonStatus, thirdPersonEnabled ?
            "Active; custom built-in TPS camera" : "Disabled; original offsets restored");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(thirdPersonStatus, "Native TPS transition failed");
        return false;
    }
}

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

static float NormalizeAngle360(float angle)
{
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

static float NormalizeAngle180(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static void FixAntiAimMovement(uintptr_t command, float realYaw, float fakeYaw)
{
    if (!command) return;
    __try {
        // blr.bzxz/bzya are the exact movement axes at +0x10/+0x14.
        const float horizontal = *reinterpret_cast<float*>(command + 0x10);
        const float vertical = *reinterpret_cast<float*>(command + 0x14);
        if (horizontal == 0.0f && vertical == 0.0f) return;
        float delta = NormalizeAngle360(fakeYaw) - NormalizeAngle360(realYaw);
        if (delta > 180.0f) delta -= 360.0f;
        else if (delta < -180.0f) delta += 360.0f;
        const float radians = delta * 0.017453292519943295f;
        const float cosine = cosf(radians);
        const float sine = sinf(radians);
        float fixedHorizontal = cosine * horizontal - sine * vertical;
        float fixedVertical = sine * horizontal + cosine * vertical;
        const float length = sqrtf(fixedHorizontal * fixedHorizontal + fixedVertical * fixedVertical);
        if (length > 1.0f) {
            fixedHorizontal /= length;
            fixedVertical /= length;
        }
        *reinterpret_cast<float*>(command + 0x10) = fixedHorizontal;
        *reinterpret_cast<float*>(command + 0x14) = fixedVertical;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static bool CaptureSilentAntiAimInput(uintptr_t player, uintptr_t command)
{
    // Read-only access to live AimingData. Fake angles are never written here.
    if (!silentAntiAimEnabled || !player || !command) return false;
    __try {
        const uintptr_t aimController =
            *reinterpret_cast<uintptr_t*>(player + 0xC8);
        const uintptr_t liveAimingData = aimController ?
            *reinterpret_cast<uintptr_t*>(aimController + 0x88) : 0;
        if (!aimController || !liveAimingData) {
            strcpy_s(silentAntiAimStatus, "Input active; waiting for live aim data");
            return false;
        }

        silentAntiAimLatestRealAimAngle =
            *reinterpret_cast<Vector3*>(liveAimingData + 0x18);
        silentAntiAimLatestRealAimEuler =
            *reinterpret_cast<Vector3*>(liveAimingData + 0x24);
        silentAntiAimLatestFiring =
            *reinterpret_cast<bool*>(command + 0x21);
        silentAntiAimLatestPlayer = player;
        silentAntiAimLatestInputValid = true;
        liveHudLocalPlayer = player;

        // Detached snapshots never enter local MovementController, so command
        // movement is already correct relative to the real camera. Rotating it
        // here by 180 degrees reverses W/S and A/D.
        strcpy_s(silentAntiAimStatus,
            "Input captured; local movement untouched");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        silentAntiAimLatestInputValid = false;
        strcpy_s(silentAntiAimStatus, "Input called; live aim read failed safely");
        return false;
    }
}

uintptr_t __fastcall hk_KeyboardControl_BuildCommand(
    uintptr_t keyboardControl, uintptr_t player, const Il2CppMethod* method)
{
    const uintptr_t command = o_KeyboardControl_BuildCommand(
        keyboardControl, player, method);
    InterlockedIncrement(&silentAntiAimInputBuildCalls);
    if (silentAntiAimEnabled && player && command)
        CaptureSilentAntiAimInput(player, command);
    return command;
}

uintptr_t __fastcall hk_AimController_GetSnapshot(
    uintptr_t aimController, const Il2CppMethod* method)
{
    const uintptr_t snapshot =
        o_AimController_GetSnapshot(aimController, method);
    InterlockedIncrement(&silentAntiAimSnapshotCalls);
    if (!silentAntiAimEnabled || !snapshot || !aimController ||
        !silentAntiAimLatestInputValid || !o_AimingData_Clone)
        return snapshot;

    __try {
        // AimController.catd +0xD0 owns this controller. Filter strictly to the
        // same local player seen by KeyboardControl.bbft.
        const uintptr_t ownerPlayer =
            *reinterpret_cast<uintptr_t*>(aimController + 0xD0);
        if (!ownerPlayer || ownerPlayer != silentAntiAimLatestPlayer)
            return snapshot;

        const uintptr_t snapshotAimingData =
            *reinterpret_cast<uintptr_t*>(snapshot + 0x18);
        if (!snapshotAimingData) return snapshot;

        // Native AimingData.rjr() creates an independent object. Repoint only
        // this AimSnapshot; live AimController.aimingData remains untouched.
        const uintptr_t detachedAimingData =
            o_AimingData_Clone(snapshotAimingData, nullptr);
        if (!detachedAimingData || detachedAimingData == snapshotAimingData) {
            strcpy_s(silentAntiAimStatus, "Snapshot active; aim clone unavailable");
            return snapshot;
        }
        *reinterpret_cast<uintptr_t*>(snapshot + 0x18) = detachedAimingData;

        Vector3 outputAimAngle = silentAntiAimLatestRealAimAngle;
        Vector3 outputAimEuler = silentAntiAimLatestRealAimEuler;
        if (!silentAntiAimLatestFiring) {
            const float fakeYaw = NormalizeAngle360(
                silentAntiAimLatestRealAimAngle.y + 180.0f);
            outputAimAngle.x = 70.0f;
            outputAimAngle.y = fakeYaw;
            outputAimEuler.x = 70.0f;
            outputAimEuler.y = fakeYaw;
        }
        *reinterpret_cast<Vector3*>(detachedAimingData + 0x18) = outputAimAngle;
        *reinterpret_cast<Vector3*>(detachedAimingData + 0x24) = outputAimEuler;
        InterlockedIncrement(&silentAntiAimAppliedCalls);
        strcpy_s(silentAntiAimStatus, silentAntiAimLatestFiring ?
            "Detached snapshot: real aim while firing" :
            "Detached snapshot: backward + down");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(silentAntiAimStatus, "Snapshot called; detached mutation failed");
    }
    return snapshot;
}

void __fastcall hk_PlayerController_Command(
    uintptr_t player, uintptr_t command, float deltaTime, const Il2CppMethod* method)
{
    InterlockedIncrement(&silentAntiAimCommandCalls);
    o_PlayerController_Command(player, command, deltaTime, method);
}

static uintptr_t GetAuthoritativeLocalWeaponController();
static uintptr_t GetCurrentLocalWeaponController();
static void UpdateWeaponChams(uintptr_t knownWeaponController = 0);
static void UpdateArmChams(uintptr_t knownArmsLodGroup = 0);
static void UpdateGloveChams(uintptr_t knownArmsLodGroup = 0);

static Vector3 DirectionToCameraEuler(const Vector3& direction)
{
    const float horizontal = sqrtf(direction.x * direction.x + direction.z * direction.z);
    const float pitch = -atan2f(direction.y, horizontal) * 57.29577951308232f;
    const float yaw = atan2f(direction.x, direction.z) * 57.29577951308232f;
    return Vector3(NormalizeAngle360(pitch), NormalizeAngle360(yaw), 0.0f);
}

static void BeginVisibleAimbotCameraSnap(const Vector3& targetDirection)
{
    if (!visibleAimbotEnabled || !liveHudLocalPlayer) return;
    __try {
        const uintptr_t aimController = *reinterpret_cast<uintptr_t*>(liveHudLocalPlayer + 0xC8);
        const uintptr_t aimingData = aimController ? *reinterpret_cast<uintptr_t*>(aimController + 0x88) : 0;
        if (!aimingData) return;
        visibleAimbotAimingData = aimingData;
        visibleAimbotOriginalAimAngle = *reinterpret_cast<Vector3*>(aimingData + 0x18);
        visibleAimbotOriginalAimEuler = *reinterpret_cast<Vector3*>(aimingData + 0x24);
        visibleAimbotTargetAim = DirectionToCameraEuler(targetDirection);
        visibleAimbotRestoreAt = GetTickCount64() + static_cast<ULONGLONG>(visibleAimbotHoldMs);
        visibleAimbotCameraActive = true;
        *reinterpret_cast<Vector3*>(aimingData + 0x18) = visibleAimbotTargetAim;
        *reinterpret_cast<Vector3*>(aimingData + 0x24) = visibleAimbotTargetAim;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { visibleAimbotCameraActive = false; visibleAimbotAimingData = 0; }
}

static void UpdateVisibleAimbotCamera()
{
    if (!visibleAimbotCameraActive || !visibleAimbotAimingData) return;
    __try {
        if (GetTickCount64() < visibleAimbotRestoreAt) {
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x18) = visibleAimbotTargetAim;
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x24) = visibleAimbotTargetAim;
        } else {
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x18) = visibleAimbotOriginalAimAngle;
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x24) = visibleAimbotOriginalAimEuler;
            visibleAimbotCameraActive = false;
            visibleAimbotAimingData = 0;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { visibleAimbotCameraActive = false; visibleAimbotAimingData = 0; }
}

static void CancelVisibleAimbotCamera()
{
    if (!visibleAimbotCameraActive) return;
    __try {
        if (visibleAimbotAimingData) {
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x18) = visibleAimbotOriginalAimAngle;
            *reinterpret_cast<Vector3*>(visibleAimbotAimingData + 0x24) = visibleAimbotOriginalAimEuler;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    visibleAimbotCameraActive = false;
    visibleAimbotAimingData = 0;
}

void __fastcall hk_HUDView_Update(uintptr_t instance, const Il2CppMethod* method)
{
    __try {
        liveAimView = instance ? *(uintptr_t*)(instance + 0x38) : 0;
        liveHitMarkerView = instance ? *(uintptr_t*)(instance + 0x48) : 0;
        liveHudLocalPlayer = instance ? *(uintptr_t*)(instance + 0xD0) : 0;
        sniperSightObject = liveAimView ? *(uintptr_t*)(liveAimView + 0x48) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { liveAimView = 0; liveHitMarkerView = 0; liveHudLocalPlayer = 0; sniperSightObject = 0; }
    o_HUDView_Update(instance, method);
    UpdateVisibleAimbotCamera();
    if (thirdPersonEnabled && !InterlockedCompareExchange(&pendingThirdPersonCommand, 0, 0))
        ApplyCustomizedThirdPersonOffsets();
    if (InterlockedCompareExchange(&pendingThirdPersonCommand, 0, 0) && ApplyNativeThirdPersonState())
        InterlockedExchange(&pendingThirdPersonCommand, 0);
    ApplyScopeOverlayState();
}

static bool ProjectTracerEnd(const Vector3& pos, ImVec2& screen);

static bool ResolveConfirmedPlayerImpact(Vector3& impactPoint, Vector3& castEndFallback)
{
    Vector3 castStart, castEnd;
    ULONGLONG castAt = 0;
    AcquireSRWLockShared(&hitMarkerLock);
    castStart = latestHitMarkerCastStart;
    castEnd = latestHitMarkerCastEnd;
    castAt = latestHitMarkerCastAt;
    const bool castValid = latestHitMarkerCastValid;
    ReleaseSRWLockShared(&hitMarkerLock);

    const ULONGLONG now = GetTickCount64();
    if (!castValid || !castAt || now < castAt || now - castAt > 2000) return false;
    castEndFallback = castEnd;
    const Vector3 segment = castEnd - castStart;
    const float segmentLengthSquared = segment.x * segment.x + segment.y * segment.y + segment.z * segment.z;
    if (segmentLengthSquared <= 0.0001f) return false;

    void* players[64];
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    void* localPlayer = GetLocalPC();
    bool found = false;
    float bestDistance = 6.0f;
    Vector3 bestPoint;

    for (int i = 0; i < playerCount; ++i) {
        if (!players[i] || players[i] == localPlayer) continue;
        Vector3 anchors[6];
        int anchorCount = 0;
        Vector3 playerAnchor;
        if (GetPCPosition(players[i], playerAnchor)) anchors[anchorCount++] = playerAnchor;
        __try {
            const uintptr_t bipedMap = *reinterpret_cast<uintptr_t*>(
                reinterpret_cast<uintptr_t>(players[i]) + 0x118);
            if (bipedMap && o_Transform_get_position) {
                const uintptr_t boneOffsets[] = { 0x20, 0x28, 0x30, 0x38, 0x40 };
                for (uintptr_t boneOffset : boneOffsets) {
                    const uintptr_t bone = *reinterpret_cast<uintptr_t*>(bipedMap + boneOffset);
                    if (bone && anchorCount < 6)
                        anchors[anchorCount++] = o_Transform_get_position(bone);
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        for (int anchorIndex = 0; anchorIndex < anchorCount; ++anchorIndex) {
            const Vector3 toAnchor = anchors[anchorIndex] - castStart;
            float t = (toAnchor.x * segment.x + toAnchor.y * segment.y + toAnchor.z * segment.z) /
                segmentLengthSquared;
            if (t < 0.0f || t > 1.05f) continue;
            const Vector3 point(
                castStart.x + segment.x * t,
                castStart.y + segment.y * t,
                castStart.z + segment.z * t);
            const float distance = point.Distance(anchors[anchorIndex]);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestPoint = point;
                found = true;
            }
        }
    }
    if (found) impactPoint = bestPoint;
    return found;
}

void __fastcall hk_HitMarkerView_Show(uintptr_t instance, bool value, bool playSound, const Il2CppMethod* method)
{
    // HitMarkerView.bbcx is the game's confirmed marker-display path. Restrict the
    // overlay to the marker owned by the active local HUD; remote HUD instances and
    // raw world/surface casts cannot trigger it.
    if (keyValidated && hitMarkerEnabled && instance && instance == liveHitMarkerView) {
        const ULONGLONG now = GetTickCount64();
        InterlockedIncrement(&hitMarkerCalls);
        Vector3 impactPoint, castEndFallback;
        const bool resolved = ResolveConfirmedPlayerImpact(impactPoint, castEndFallback);
        bool haveWorldFallback = false;
        AcquireSRWLockShared(&hitMarkerLock);
        haveWorldFallback = latestHitMarkerCastValid && latestHitMarkerCastAt &&
            now >= latestHitMarkerCastAt && now - latestHitMarkerCastAt <= 2000;
        ReleaseSRWLockShared(&hitMarkerLock);

        AcquireSRWLockExclusive(&hitMarkerLock);
        // A confirmed hit must always create a visible marker. Prefer the player
        // intersection; if runtime player enumeration is unavailable, keep a visible
        // diagnostic fallback rather than silently dropping the confirmed hit.
        hitMarkers.push_back({ resolved ? impactPoint : castEndFallback, now,
            !resolved && !haveWorldFallback });
        if (hitMarkers.size() > 64)
            hitMarkers.erase(hitMarkers.begin(), hitMarkers.begin() + (hitMarkers.size() - 64));
        ReleaseSRWLockExclusive(&hitMarkerLock);
        InterlockedIncrement(resolved ? &hitMarkerResolvedCalls : &hitMarkerFallbackCalls);
    }
    o_HitMarkerView_Show(instance, value, playSound, method);
}

void DrawHitMarker()
{
    if (!keyValidated || !hitMarkerEnabled) return;

    const ULONGLONG now = GetTickCount64();
    const float durationMs = hitMarkerDuration * 1000.0f;
    std::vector<HitMarkerEntry> snapshot;
    AcquireSRWLockExclusive(&hitMarkerLock);
    hitMarkers.erase(std::remove_if(hitMarkers.begin(), hitMarkers.end(), [now, durationMs](const HitMarkerEntry& marker) {
        return now < marker.triggeredAt || static_cast<float>(now - marker.triggeredAt) >= durationMs;
    }), hitMarkers.end());
    snapshot = hitMarkers;
    ReleaseSRWLockExclusive(&hitMarkerLock);

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    for (const HitMarkerEntry& marker : snapshot) {
        const float elapsedMs = static_cast<float>(now - marker.triggeredAt);
        const float alpha = 1.0f - elapsedMs / durationMs;
        ImVec2 center;
        if (marker.screenCenterFallback) {
            const ImVec2 display = ImGui::GetIO().DisplaySize;
            center = ImVec2(display.x * 0.5f, display.y * 0.5f);
        }
        else if (!ProjectTracerEnd(marker.worldPosition, center)) {
            continue;
        }

        const ImU32 shadow = IM_COL32(0, 0, 0, static_cast<int>(190.0f * alpha));
        const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(
            hitMarkerColor[0], hitMarkerColor[1], hitMarkerColor[2], alpha));
        const float inner = hitMarkerGap;
        const float outer = hitMarkerGap + hitMarkerSize;
        const ImVec2 starts[4] = {
            ImVec2(center.x - inner, center.y - inner), ImVec2(center.x + inner, center.y - inner),
            ImVec2(center.x - inner, center.y + inner), ImVec2(center.x + inner, center.y + inner)
        };
        const ImVec2 ends[4] = {
            ImVec2(center.x - outer, center.y - outer), ImVec2(center.x + outer, center.y - outer),
            ImVec2(center.x - outer, center.y + outer), ImVec2(center.x + outer, center.y + outer)
        };
        for (int i = 0; i < 4; ++i) {
            draw->AddLine(starts[i], ends[i], shadow, hitMarkerThickness + 2.0f);
            draw->AddLine(starts[i], ends[i], color, hitMarkerThickness);
        }
    }
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

static bool GetAimbotHead(void* player, Vector3& headPosition)
{
    if (!player || !o_Transform_get_position) return false;
    __try {
        const uintptr_t biped = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(player) + 0x118);
        const uintptr_t head = biped ? *reinterpret_cast<uintptr_t*>(biped + 0x20) : 0;
        if (!head) return false;
        headPosition = o_Transform_get_position(head);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool FindVisibleAimbotDirection(Vector3 origin, uintptr_t hitParameters, Vector3& outDirection)
{
    void* localPlayer = GetLocalPC();
    if (!localPlayer || !o_HitCaster_Cast) return false;
    const unsigned char localTeam = GetPCTeam(localPlayer);
    if (localTeam == 0 || localTeam == 3) return false;

    void* players[64] = {};
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    InterlockedExchange(&aimbotTargetsScanned, playerCount);

    bool found = false;
    int visibleCount = 0;
    float bestDistance = 3.402823466e+38F;
    Vector3 bestDirection;
    for (int i = 0; i < playerCount; ++i) {
        void* player = players[i];
        if (!player || player == localPlayer) continue;
        const unsigned char team = GetPCTeam(player);
        if (team == 0 || team == 3 || team == localTeam) continue;

        Vector3 head;
        if (!GetAimbotHead(player, head)) continue;
        const Vector3 delta(head.x - origin.x, head.y - origin.y, head.z - origin.z);
        const float distance = delta.Length();
        if (distance < 0.5f) continue;
        const Vector3 candidate(delta.x / distance, delta.y / distance, delta.z / distance);

        bool visible = !aimbotVisibleCheck;
        if (aimbotVisibleCheck) {
            __try {
                // Use the game's own bullet caster as the visibility test. Stop the
                // probe just behind the head: unobstructed rays finish within 0.6 m of
                // the target; walls stop the cast significantly earlier.
                const uintptr_t probe = o_HitCaster_Cast(origin, candidate, distance + 0.35f, hitParameters, nullptr);
                if (probe) {
                    const Vector3 probeEnd = *reinterpret_cast<Vector3*>(probe + 0x30);
                    visible = probeEnd.Distance(head) <= 0.60f;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { visible = false; }
        }
        if (!visible) continue;
        ++visibleCount;
        // 360-degree FOV: no screen-angle rejection. Select the nearest visible enemy.
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDirection = candidate;
            found = true;
        }
    }
    InterlockedExchange(&aimbotVisibleTargets, visibleCount);
    if (found) outDirection = bestDirection;
    return found;
}

static bool ScanIndependentVisibleAimbotDirection(Vector3 origin, Vector3& outDirection)
{
    void* localPlayer = GetLocalPC();
    if (!localPlayer || !base) return false;
    const unsigned char localTeam = GetPCTeam(localPlayer);
    if (localTeam == 0 || localTeam == 3) return false;

    using RaycasterFn = bool(__fastcall*)(Vector3, Vector3, float, NativeRaycastHit*, Il2CppArray*, const Il2CppMethod*);
    static RaycasterFn raycaster = nullptr;
    static Il2CppArray* emptyStringFilters = nullptr;
    if (!raycaster) raycaster = reinterpret_cast<RaycasterFn>(base + OFFSET_RAYCASTER_QMK);
    if (!emptyStringFilters && g_il2cpp.array_new) {
        Il2CppClass* stringClass = g_il2cpp.find_class("System", "String");
        if (stringClass) emptyStringFilters = g_il2cpp.array_new(stringClass, 0);
    }

    void* players[64] = {};
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    InterlockedExchange(&aimbotTargetsScanned, playerCount);
    bool found = false;
    int visibleCount = 0;
    float bestDistance = 3.402823466e+38F;
    Vector3 bestDirection;
    for (int i = 0; i < playerCount; ++i) {
        void* player = players[i];
        if (!player || player == localPlayer) continue;
        const unsigned char team = GetPCTeam(player);
        if (team == 0 || team == 3 || team == localTeam) continue;
        Vector3 head;
        if (!GetAimbotHead(player, head)) continue;
        const Vector3 delta(head.x - origin.x, head.y - origin.y, head.z - origin.z);
        const float distance = delta.Length();
        if (distance < 0.5f) continue;
        const Vector3 candidate(delta.x / distance, delta.y / distance, delta.z / distance);
        bool visible = !aimbotVisibleCheck;
        if (aimbotVisibleCheck && raycaster) {
            __try {
                NativeRaycastHit hit = {};
                const bool collided = raycaster(origin, candidate, distance + 0.35f,
                    &hit, emptyStringFilters, nullptr);
                visible = !collided || hit.point.Distance(head) <= 0.75f ||
                    (hit.distance >= distance - 0.75f && hit.distance <= distance + 0.75f);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { visible = false; }
        }
        if (!visible) continue;
        ++visibleCount;
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDirection = candidate;
            found = true;
        }
    }
    InterlockedExchange(&aimbotVisibleTargets, visibleCount);
    if (found) outDirection = bestDirection;
    return found;
}

static void RefreshAimbotVisibilityCache()
{
    const ULONGLONG now = GetTickCount64();
    if (now < aimbotVisibilityNextScanAt) return;
    aimbotVisibilityNextScanAt = now + 50; // max 20 scans/sec; never per movement callback
    InterlockedIncrement(&aimbotVisibilityScans);

    Vector3 origin;
    bool haveOrigin = false;
    __try {
        const uintptr_t camera = GetCamera();
        const uintptr_t transform = camera && o_Component_get_transform ? o_Component_get_transform(camera) : 0;
        if (transform && o_Transform_get_position) { origin = o_Transform_get_position(transform); haveOrigin = true; }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { haveOrigin = false; }
    Vector3 direction;
    const bool visible = haveOrigin && ScanIndependentVisibleAimbotDirection(origin, direction);
    aimbotCachedVisibleTarget = visible;
    if (visible) {
        aimbotCachedVisibleDirection = direction;
        aimbotVisibilityValidUntil = now + 90;
    } else {
        aimbotVisibilityValidUntil = now + 50;
    }
}

uintptr_t __fastcall hk_HitCaster_Cast(Vector3 origin, Vector3 direction, float maxDistance, uintptr_t hitParameters, const Il2CppMethod* method)
{
    bool applyVisibleSnapAfterCast = false;
    Vector3 visibleSnapDirection;
    if (insideLocalGunFire && hitParameters) aimbotLastHitParameters = hitParameters;
    if (keyValidated && insideLocalGunFire && (aimbotEnabled || visibleAimbotEnabled)) {
        InterlockedIncrement(&aimbotShots);
        Vector3 targetDirection;
        if (FindVisibleAimbotDirection(origin, hitParameters, targetDirection)) {
            // Silent mode keeps doing exactly what it did before. Visible mode uses
            // the same proven shot direction but also rotates the real rendered camera.
            direction = targetDirection;
            if (visibleAimbotEnabled) {
                visibleSnapDirection = targetDirection;
                applyVisibleSnapAfterCast = true;
            }
            InterlockedIncrement(&aimbotApplied);
            strcpy_s(aimbotStatus, visibleAimbotEnabled ?
                "Shot accepted; visible camera snap applied" :
                "Silent lock applied; camera untouched");
        } else {
            strcpy_s(aimbotStatus, "No visible enemy; original shot kept");
        }
    }
    else if (keyValidated && noSpreadEnabled && insideLocalGunFire) {
        __try {
            const uintptr_t camera = GetCamera();
            const uintptr_t cameraTransform = camera && o_Component_get_transform ? o_Component_get_transform(camera) : 0;
            if (cameraTransform && o_Transform_get_forward)
                direction = o_Transform_get_forward(cameraTransform);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    const uintptr_t result = o_HitCaster_Cast(origin, direction, maxDistance, hitParameters, method);
    // Never mutate the live aim state before/during Fire. Only animate the camera
    // after the game has accepted and completed the real bullet cast.
    if (result && applyVisibleSnapAfterCast)
        BeginVisibleAimbotCameraSnap(visibleSnapDirection);
    if (keyValidated && insideLocalGunFire && result && (hitMarkerEnabled || bulletTracerEnabled)) {
        __try {
            // cjr stores the authoritative cast start/end at 0x24/0x30.
            const Vector3 actualStart = *(Vector3*)(result + 0x24);
            const Vector3 actualEnd = *(Vector3*)(result + 0x30);
            const ULONGLONG now = GetTickCount64();
            if (hitMarkerEnabled) {
                AcquireSRWLockExclusive(&hitMarkerLock);
                latestHitMarkerCastStart = actualStart;
                latestHitMarkerCastEnd = actualEnd;
                latestHitMarkerCastAt = now;
                latestHitMarkerCastValid = true;
                ReleaseSRWLockExclusive(&hitMarkerLock);
            }
            if (bulletTracerEnabled) {
                AcquireSRWLockExclusive(&bulletTracerLock);
                bulletTracers.push_back({ actualStart, actualEnd, now });
                if (bulletTracers.size() > 128) bulletTracers.erase(bulletTracers.begin(), bulletTracers.begin() + (bulletTracers.size() - 128));
                ReleaseSRWLockExclusive(&bulletTracerLock);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return result;
}

void __fastcall hk_ArmsLod_SetVisible(uintptr_t instance, bool visible, const Il2CppMethod* method)
{
    o_ArmsLod_SetVisible(instance, visible, method);
    uintptr_t localArms = 0;
    __try { localArms = liveHudLocalPlayer ? *(uintptr_t*)(liveHudLocalPlayer + 0x138) : 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { localArms = 0; }
    if (!instance || !visible || instance != localArms) return;
    armChamsArmsLodGroup = instance;
    gloveChamsArmsLodGroup = instance;
    if (armChamsEnabled) InterlockedExchange(&pendingArmChamsRefresh, 1);
    if (gloveChamsEnabled) InterlockedExchange(&pendingGloveChamsRefresh, 1);
}

void __fastcall hk_Gloves_SetArms(uintptr_t instance, uintptr_t armsLodGroup, const Il2CppMethod* method)
{
    o_Gloves_SetArms(instance, armsLodGroup, method);
    uintptr_t localArms = 0;
    __try { localArms = liveHudLocalPlayer ? *(uintptr_t*)(liveHudLocalPlayer + 0x138) : 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { localArms = 0; }
    if (!armsLodGroup || armsLodGroup != localArms) return;
    armChamsArmsLodGroup = armsLodGroup;
    gloveChamsArmsLodGroup = armsLodGroup;
    if (armChamsEnabled && armsLodGroup) InterlockedExchange(&pendingArmChamsRefresh, 1);
    if (gloveChamsEnabled && armsLodGroup) InterlockedExchange(&pendingGloveChamsRefresh, 1);
}

void __fastcall hk_Weaponry_TakeWeapon(uintptr_t instance, uint8_t slotIndex, const Il2CppMethod* method)
{
    o_Weaponry_TakeWeapon(instance, slotIndex, method);
    uintptr_t weaponController = 0;
    bool isLocalWeaponry = false;
    __try {
        const uintptr_t ownerPlayer = instance ? *(uintptr_t*)(instance + 0x60) : 0;
        isLocalWeaponry = ownerPlayer && ownerPlayer == liveHudLocalPlayer;
        weaponController = isLocalWeaponry ? *(uintptr_t*)(instance + OFFSET_WEAPONCONTROLLER) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { weaponController = 0; isLocalWeaponry = false; }
    if (!isLocalWeaponry) return;
    activeLocalWeaponController = weaponController;
    if (weaponChamsEnabled && weaponController) {
        sprintf_s(weaponChamsStatus, "Slot %u equipped; queued", static_cast<unsigned>(slotIndex));
        InterlockedExchange(&pendingWeaponChamsRefresh, 1);
    }
}

void __fastcall hk_GunController_Fire(uintptr_t instance, Vector3 playSound, const Il2CppMethod* method)
{
    // Fire must remain latency-free: never create materials, inspect renderers,
    // or call set_material from this hook. Only remember the live gun and queue
    // maintenance for the normal movement/game loop after the shot.
    bool isLocalGun = false;
    __try {
        const uintptr_t currentWeapon = GetCurrentLocalWeaponController();
        const uintptr_t owner = instance ? *reinterpret_cast<uintptr_t*>(instance + 0x20) : 0;
        isLocalGun = instance && ((currentWeapon && instance == currentWeapon) ||
            (liveHudLocalPlayer && owner == liveHudLocalPlayer));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { isLocalGun = false; }
    if (isLocalGun) activeLocalWeaponController = instance;
    if (isLocalGun && infinityAmmo) InterlockedIncrement(&infinityAmmoFireCalls);
    short ammoBeforeShot = -1;
    if (isLocalGun && infinityAmmo) {
        __try {
            ammoBeforeShot = *reinterpret_cast<short*>(instance + OFFSET_CURRENT_AMMO);
            if (ammoBeforeShot >= 0 && ammoBeforeShot < 1000) {
                frozenAmmoWeapon = instance;
                frozenAmmoValue = ammoBeforeShot;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { ammoBeforeShot = -1; }
    }
    insideLocalGunFire = isLocalGun;
    o_GunController_Fire(instance, playSound, method);
    insideLocalGunFire = false;
    if (isLocalGun && infinityAmmo && ammoBeforeShot >= 0) {
        __try {
            *reinterpret_cast<short*>(instance + OFFSET_CURRENT_AMMO) = ammoBeforeShot;
            using SetAmmoFn = void(__fastcall*)(uintptr_t, short, const Il2CppMethod*);
            reinterpret_cast<SetAmmoFn>(base + 0x999190)(instance, ammoBeforeShot, nullptr);
            *reinterpret_cast<short*>(instance + OFFSET_CURRENT_AMMO) = ammoBeforeShot;
            InterlockedExchange(&infinityAmmoLastField, ammoBeforeShot);
            InterlockedIncrement(&infinityAmmoRestores);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    if (isLocalGun && weaponChamsEnabled) InterlockedExchange(&pendingWeaponChamsRefresh, 1);
}

static void UpdateAimbotAutoFire()
{
    if (!keyValidated || !aimbotAutoFire || (!aimbotEnabled && !visibleAimbotEnabled) ||
        !activeLocalWeaponController || !o_GunController_Fire) {
        aimbotCachedVisibleTarget = false;
        return;
    }
    RefreshAimbotVisibilityCache();
    const ULONGLONG now = GetTickCount64();
    if (!aimbotCachedVisibleTarget || now > aimbotVisibilityValidUntil) return;

    InterlockedIncrement(&aimbotAutoFired);
    // No software delay: call every movement update while the 20 Hz visibility
    // cache is fresh. GunController enforces the weapon's native fire rate.
    hk_GunController_Fire(activeLocalWeaponController, Vector3(), nullptr);
}

short __fastcall hk_GunController_GetCurrentAmmo(uintptr_t instance)
{
    const short nativeAmmo = o_GunController_GetCurrentAmmo(instance);
    if (!keyValidated || !instance) return nativeAmmo;
    bool isLocalGun = false;
    __try {
        const uintptr_t currentWeapon = GetCurrentLocalWeaponController();
        const uintptr_t owner = *reinterpret_cast<uintptr_t*>(instance + 0x20);
        isLocalGun = (currentWeapon && instance == currentWeapon) ||
            (liveHudLocalPlayer && owner == liveHudLocalPlayer);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { isLocalGun = false; }
    if (!isLocalGun) return nativeAmmo;
    activeLocalWeaponController = instance;
    InterlockedIncrement(&infinityAmmoGetterCalls);
    InterlockedExchange(&infinityAmmoLastGetter, nativeAmmo);
    if (infinityAmmo) {
        if (instance != frozenAmmoWeapon || frozenAmmoValue < 0) {
            frozenAmmoWeapon = instance;
            frozenAmmoValue = nativeAmmo;
        }
        if (frozenAmmoValue >= 0 && frozenAmmoValue < 1000) return frozenAmmoValue;
    }
    return nativeAmmo;
}



void InfinityAmmoLoop()
{
    if (!keyValidated || !base) return;
    __try {
        const uintptr_t weaponController = GetCurrentLocalWeaponController();
        if (!weaponController) return;
        if (!infinityAmmo) {
            frozenAmmoWeapon = 0;
            frozenAmmoValue = -1;
            return;
        }
        const short currentAmmo = *reinterpret_cast<short*>(weaponController + OFFSET_CURRENT_AMMO);
        InterlockedExchange(&infinityAmmoLastField, currentAmmo);
        if (weaponController != frozenAmmoWeapon || frozenAmmoValue < 0) {
            const short nativeAmmo = o_GunController_GetCurrentAmmo ?
                o_GunController_GetCurrentAmmo(weaponController) : currentAmmo;
            if (nativeAmmo >= 0 && nativeAmmo < 1000) {
                frozenAmmoWeapon = weaponController;
                frozenAmmoValue = nativeAmmo;
            }
        }
        if (frozenAmmoValue >= 0 && frozenAmmoValue < 1000) {
            using SetAmmoFn = void(__fastcall*)(uintptr_t, short, const Il2CppMethod*);
            reinterpret_cast<SetAmmoFn>(base + 0x999190)(weaponController, frozenAmmoValue, nullptr);
            *reinterpret_cast<short*>(weaponController + OFFSET_CURRENT_AMMO) = frozenAmmoValue;
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
    if (!keyValidated) return vel;

    if (jbActive) {
        const float horSpeed = sqrtf(vel.x * vel.x + vel.z * vel.z);
        if (horSpeed < surfSpeed * 3.0f) {
            vel.x *= 1.05f;
            vel.z *= 1.05f;
        }
        vel.y = 0.0f;
    }

    // This getter can be consumed by game movement code, but the authoritative
    // physical cap is also enforced on CharacterController.Move below.
    if (velocityLimiterEnabled) vel = ApplyVelocityLimit(vel);
    velocityLimiterCurrentHorizontalSpeed = sqrtf(vel.x * vel.x + vel.z * vel.z);
    lastSpeed = sqrtf(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
    return vel;
}

static Vector3 ApplyVelocityLimitToMotion(const Vector3& motion)
{
    velocityLimiterLastAppliedScale = 1.0f;
    if (!velocityLimiterEnabled || velocityLimit <= 0.0f) return motion;

    float deltaTime = o_Time_get_deltaTime ? o_Time_get_deltaTime() : (1.0f / 60.0f);
    if (!isfinite(deltaTime) || deltaTime < (1.0f / 240.0f) || deltaTime > 0.1f)
        deltaTime = 1.0f / 60.0f;

    Vector3 result = motion;
    const float horizontalMotion = sqrtf(result.x * result.x + result.z * result.z);
    const float maxHorizontalMotion = velocityLimit * deltaTime;
    if (horizontalMotion > maxHorizontalMotion && horizontalMotion > 0.00001f) {
        velocityLimiterLastAppliedScale = maxHorizontalMotion / horizontalMotion;
        result.x *= velocityLimiterLastAppliedScale;
        result.z *= velocityLimiterLastAppliedScale;
    }
    return result;
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
    const int mode = (weaponChamsMode >= 0 && weaponChamsMode < 7) ? weaponChamsMode : 0;
    if (weaponChamsMaterials[mode]) return weaponChamsMaterials[mode];

    static const char* shaderCandidates[7][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        weaponChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (weaponChamsMaterials[mode]) break;
    }
    const uintptr_t material = weaponChamsMaterials[mode];
    if (!material) {
        static const char* names[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse" };
        sprintf_s(weaponChamsStatus, "%s shader not found in this build", names[mode]);
        return 0;
    }
    if (g_il2cpp.gchandle_new)
        weaponChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    if (mode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(material, g_il2cpp.string_new("_Metallic"), weaponChamsMetallic);
        o_Material_SetFloat(material, g_il2cpp.string_new("_Glossiness"), weaponChamsSmoothness);
    }
    return material;
}

static Color GetWeaponChamsDisplayColor()
{
    if (weaponChamsMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * weaponChamsAnimationSpeed;
        const float r = 0.5f + 0.5f * sinf(t);
        const float g = 0.5f + 0.5f * sinf(t + 2.0943951f);
        const float b = 0.5f + 0.5f * sinf(t + 4.1887902f);
        return Color(r, g, b, 1.0f);
    }
    if (weaponChamsMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * weaponChamsAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(weaponChamsColor[0] * intensity, weaponChamsColor[1] * intensity, weaponChamsColor[2] * intensity, 1.0f);
    }
    const float alpha = (weaponChamsMode == 1 || weaponChamsMode == 4) ? weaponChamsGlassAlpha : 1.0f;
    return Color(weaponChamsColor[0], weaponChamsColor[1], weaponChamsColor[2], alpha);
}

static void AnimateWeaponChamsColor()
{
    if (!weaponChamsEnabled || (weaponChamsMode != 5 && weaponChamsMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - weaponChamsLastAnimationTick < 33) return;
    weaponChamsLastAnimationTick = now;
    const uintptr_t material = weaponChamsMaterials[weaponChamsMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetWeaponChamsDisplayColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uintptr_t EnsureSelectedArmChamsMaterial()
{
    const int mode = (armChamsMode >= 0 && armChamsMode < 7) ? armChamsMode : 0;
    if (armChamsMaterials[mode]) return armChamsMaterials[mode];
    static const char* shaderCandidates[7][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        armChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (armChamsMaterials[mode]) break;
    }
    const uintptr_t material = armChamsMaterials[mode];
    if (!material) { strcpy_s(armChamsStatus, "Selected arm shader not found"); return 0; }
    if (g_il2cpp.gchandle_new)
        armChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    return material;
}

static Color GetArmChamsDisplayColor()
{
    if (armChamsMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * armChamsAnimationSpeed;
        return Color(0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.0943951f), 0.5f + 0.5f * sinf(t + 4.1887902f), 1.0f);
    }
    if (armChamsMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * armChamsAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(armChamsColor[0] * intensity, armChamsColor[1] * intensity, armChamsColor[2] * intensity, 1.0f);
    }
    return Color(armChamsColor[0], armChamsColor[1], armChamsColor[2], (armChamsMode == 1 || armChamsMode == 4) ? armChamsAlpha : 1.0f);
}

static void AnimateArmChamsColor()
{
    if (!armChamsEnabled || (armChamsMode != 5 && armChamsMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - armChamsLastAnimationTick < 33) return;
    armChamsLastAnimationTick = now;
    const uintptr_t material = armChamsMaterials[armChamsMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetArmChamsDisplayColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uintptr_t GetCurrentLocalArmsLodGroup()
{
    __try {
        const uintptr_t localArms = liveHudLocalPlayer ? *(uintptr_t*)(liveHudLocalPlayer + 0x138) : 0;
        if (localArms) {
            armChamsArmsLodGroup = localArms;
            gloveChamsArmsLodGroup = localArms;
            return localArms;
        }
        return armChamsArmsLodGroup ? armChamsArmsLodGroup : gloveChamsArmsLodGroup;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return armChamsArmsLodGroup ? armChamsArmsLodGroup : gloveChamsArmsLodGroup;
    }
}

static void RestoreArmChams()
{
    if (o_Renderer_set_material) {
        for (const ArmChamsRenderer& entry : armChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            __try { o_Renderer_set_material(entry.renderer, entry.originalMaterial); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    armChamsRenderers.clear();
}

static void CaptureArmChamsRenderers(uintptr_t armsLodGroup)
{
    RestoreArmChams();
    if (!armsLodGroup || !o_Renderer_get_material) { strcpy_s(armChamsStatus, "Local arm renderer not found"); return; }
    __try {
        // ArmsLodGroup::_armsMeshRenderer at +0x60. Gloves are handled separately.
        const uintptr_t renderer = *(uintptr_t*)(armsLodGroup + 0x60);
        if (renderer) {
            const uintptr_t originalMaterial = o_Renderer_get_material(renderer);
            if (originalMaterial) armChamsRenderers.push_back({ renderer, originalMaterial });
        }
        armChamsArmsLodGroup = armsLodGroup;
        sprintf_s(armChamsStatus, "Applied to %zu arm renderer(s)", armChamsRenderers.size());
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(armChamsStatus, "Arm renderer capture failed");
        RestoreArmChams();
    }
}

static void UpdateArmChams(uintptr_t knownArmsLodGroup)
{
    if (!keyValidated || !o_Renderer_set_material || !o_Material_set_color) return;
    uintptr_t armsLodGroup = knownArmsLodGroup ? knownArmsLodGroup : armChamsArmsLodGroup;
    if (!armsLodGroup) armsLodGroup = GetCurrentLocalArmsLodGroup();
    if (!armChamsEnabled) {
        if (!armChamsRenderers.empty()) RestoreArmChams();
        strcpy_s(armChamsStatus, "Disabled");
        return;
    }
    if (!armsLodGroup) { strcpy_s(armChamsStatus, "Waiting for local arms"); return; }
    const uintptr_t replacement = EnsureSelectedArmChamsMaterial();
    if (!replacement) return;
    bool rendererChanged = armsLodGroup != armChamsArmsLodGroup || armChamsRenderers.empty();
    __try {
        const uintptr_t liveRenderer = *(uintptr_t*)(armsLodGroup + 0x60);
        if (!liveRenderer || armChamsRenderers.empty() || armChamsRenderers.front().renderer != liveRenderer) rendererChanged = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { rendererChanged = true; }
    if (rendererChanged) CaptureArmChamsRenderers(armsLodGroup);
    if (armChamsRenderers.empty()) { strcpy_s(armChamsStatus, "Waiting for arm renderer"); return; }
    if (armChamsMode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), armChamsMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), armChamsSmoothness);
    }
    o_Material_set_color(replacement, GetArmChamsDisplayColor());
    for (const ArmChamsRenderer& entry : armChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
            if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(armChamsStatus, "Active");
}

static uintptr_t EnsureSelectedGloveChamsMaterial()
{
    const int mode = (gloveChamsMode >= 0 && gloveChamsMode < 7) ? gloveChamsMode : 0;
    if (gloveChamsMaterials[mode]) return gloveChamsMaterials[mode];
    static const char* shaderCandidates[7][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        gloveChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (gloveChamsMaterials[mode]) break;
    }
    const uintptr_t material = gloveChamsMaterials[mode];
    if (!material) { strcpy_s(gloveChamsStatus, "Selected glove shader not found"); return 0; }
    if (g_il2cpp.gchandle_new)
        gloveChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    return material;
}

static Color GetGloveChamsDisplayColor()
{
    if (gloveChamsMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * gloveChamsAnimationSpeed;
        return Color(0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.0943951f), 0.5f + 0.5f * sinf(t + 4.1887902f), 1.0f);
    }
    if (gloveChamsMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * gloveChamsAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(gloveChamsColor[0] * intensity, gloveChamsColor[1] * intensity, gloveChamsColor[2] * intensity, 1.0f);
    }
    return Color(gloveChamsColor[0], gloveChamsColor[1], gloveChamsColor[2], (gloveChamsMode == 1 || gloveChamsMode == 4) ? gloveChamsAlpha : 1.0f);
}

static void AnimateGloveChamsColor()
{
    if (!gloveChamsEnabled || (gloveChamsMode != 5 && gloveChamsMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - gloveChamsLastAnimationTick < 33) return;
    gloveChamsLastAnimationTick = now;
    const uintptr_t material = gloveChamsMaterials[gloveChamsMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetGloveChamsDisplayColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void RestoreGloveChams()
{
    if (o_Renderer_set_material) {
        for (const GloveChamsRenderer& entry : gloveChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            __try { o_Renderer_set_material(entry.renderer, entry.originalMaterial); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    gloveChamsRenderers.clear();
}

static void CaptureGloveChamsRenderers(uintptr_t armsLodGroup)
{
    RestoreGloveChams();
    if (!armsLodGroup || !o_Renderer_get_material) { strcpy_s(gloveChamsStatus, "Local glove renderer not found"); return; }
    __try {
        // ArmsLodGroup::_glovesMeshRenderer at +0x68.
        const uintptr_t renderer = *(uintptr_t*)(armsLodGroup + 0x68);
        if (renderer) {
            const uintptr_t originalMaterial = o_Renderer_get_material(renderer);
            if (originalMaterial) gloveChamsRenderers.push_back({ renderer, originalMaterial });
        }
        gloveChamsArmsLodGroup = armsLodGroup;
        sprintf_s(gloveChamsStatus, "Applied to %zu glove renderer(s)", gloveChamsRenderers.size());
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(gloveChamsStatus, "Glove renderer capture failed");
        RestoreGloveChams();
    }
}

static void UpdateGloveChams(uintptr_t knownArmsLodGroup)
{
    if (!keyValidated || !o_Renderer_set_material || !o_Material_set_color) return;
    uintptr_t armsLodGroup = knownArmsLodGroup ? knownArmsLodGroup : gloveChamsArmsLodGroup;
    if (!armsLodGroup) armsLodGroup = GetCurrentLocalArmsLodGroup();
    if (!gloveChamsEnabled) {
        if (!gloveChamsRenderers.empty()) RestoreGloveChams();
        strcpy_s(gloveChamsStatus, "Disabled");
        return;
    }
    if (!armsLodGroup) { strcpy_s(gloveChamsStatus, "Waiting for local gloves"); return; }
    const uintptr_t replacement = EnsureSelectedGloveChamsMaterial();
    if (!replacement) return;
    bool rendererChanged = armsLodGroup != gloveChamsArmsLodGroup || gloveChamsRenderers.empty();
    __try {
        const uintptr_t liveRenderer = *(uintptr_t*)(armsLodGroup + 0x68);
        if (!liveRenderer || gloveChamsRenderers.empty() || gloveChamsRenderers.front().renderer != liveRenderer) rendererChanged = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { rendererChanged = true; }
    if (rendererChanged) CaptureGloveChamsRenderers(armsLodGroup);
    if (gloveChamsRenderers.empty()) { strcpy_s(gloveChamsStatus, "Waiting for glove renderer"); return; }
    if (gloveChamsMode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), gloveChamsMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), gloveChamsSmoothness);
    }
    o_Material_set_color(replacement, GetGloveChamsDisplayColor());
    for (const GloveChamsRenderer& entry : gloveChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
            if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(gloveChamsStatus, "Active");
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

static uintptr_t GetAuthoritativeLocalWeaponController()
{
    __try {
        const uintptr_t weaponry = liveHudLocalPlayer ? *(uintptr_t*)(liveHudLocalPlayer + OFFSET_WEAPONRYCONTROLLER) : 0;
        return weaponry ? *(uintptr_t*)(weaponry + OFFSET_WEAPONCONTROLLER) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static uintptr_t GetCurrentLocalWeaponController()
{
    const uintptr_t currentWeapon = GetAuthoritativeLocalWeaponController();
    return currentWeapon ? currentWeapon : activeLocalWeaponController;
}

static void UpdateWeaponChams(uintptr_t knownWeaponController)
{
    if (!keyValidated) { strcpy_s(weaponChamsStatus, "License validation inactive"); return; }
    if (!o_Renderer_set_material || !o_Material_set_color) { strcpy_s(weaponChamsStatus, "Material API unavailable"); return; }
    uintptr_t weaponController = knownWeaponController ? knownWeaponController : activeLocalWeaponController;
    if (!weaponController) weaponController = GetCurrentLocalWeaponController();
    if (weaponController) activeLocalWeaponController = weaponController;

    if (!weaponChamsEnabled) {
        if (!weaponChamsRenderers.empty()) RestoreWeaponChams();
        strcpy_s(weaponChamsStatus, "Disabled");
        return;
    }
    if (!weaponController && weaponChamsRenderers.empty()) {
        strcpy_s(weaponChamsStatus, "Waiting for first weapon HUD update");
        return;
    }
    const uintptr_t replacement = EnsureSelectedWeaponChamsMaterial();
    if (!replacement) return;
    if (weaponController && (weaponController != weaponChamsController || weaponChamsRenderers.empty()))
        CaptureWeaponChamsRenderers(weaponController);
    if (weaponChamsRenderers.empty()) return;

    if (weaponChamsMode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), weaponChamsMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), weaponChamsSmoothness);
    }
    o_Material_set_color(replacement, GetWeaponChamsDisplayColor());
    for (const WeaponChamsRenderer& entry : weaponChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
            if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(weaponChamsStatus, "Active");
}

int __fastcall hk_CC_Move(uintptr_t instance, Vector3 motion)
{
    if (keyValidated && instance) lastCharacterController = instance;

    const uintptr_t currentLocalPlayer = liveHudLocalPlayer;
    const bool localPlayerChanged = currentLocalPlayer && currentLocalPlayer != chamsObservedLocalPlayer;
    if (localPlayerChanged) {
        // A different local PlayerController means a new match/respawn scene.
        // Old Unity renderer pointers may already be destroyed, so abandon them
        // without attempting Restore* on stale objects and rediscover everything.
        chamsObservedLocalPlayer = currentLocalPlayer;
        customizedThirdPersonPlayer = 0;
        originalTpsOffsetsCaptured = false;
        weaponChamsRenderers.clear();
        armChamsRenderers.clear();
        gloveChamsRenderers.clear();
        weaponChamsController = 0;
        activeLocalWeaponController = 0;
        aimbotCachedVisibleTarget = false;
        aimbotVisibilityNextScanAt = 0;
        armChamsArmsLodGroup = 0;
        gloveChamsArmsLodGroup = 0;
        if (weaponChamsEnabled) InterlockedExchange(&pendingWeaponChamsRefresh, 1);
        if (armChamsEnabled) InterlockedExchange(&pendingArmChamsRefresh, 1);
        if (gloveChamsEnabled) InterlockedExchange(&pendingGloveChamsRefresh, 1);
        InterlockedExchange(&pendingThirdPersonCommand, 1);
        worldColorRenderers.clear();
        worldColorTintMaterials.clear();
        worldColorRetryCount = 0;
        worldColorNextRetryTick = GetTickCount64() + 1500;
        if (worldColorEnabled) strcpy_s(worldColorStatus, "Waiting for new map to stabilize...");
    }

    const bool weaponRefreshRequested = InterlockedExchange(&pendingWeaponChamsRefresh, 0) != 0;
    const bool armRefreshRequested = InterlockedExchange(&pendingArmChamsRefresh, 0) != 0;
    const bool gloveRefreshRequested = InterlockedExchange(&pendingGloveChamsRefresh, 0) != 0;
    const ULONGLONG chamsNow = GetTickCount64();
    const bool maintenanceDue = chamsNow - armGloveChamsLastMaintenanceTick >= 500;
    if (weaponRefreshRequested || armRefreshRequested || gloveRefreshRequested || maintenanceDue) {
        if (maintenanceDue) armGloveChamsLastMaintenanceTick = chamsNow;
        if (weaponRefreshRequested || (maintenanceDue && weaponChamsEnabled))
            UpdateWeaponChams(GetCurrentLocalWeaponController());
        const uintptr_t liveArmsLodGroup = GetCurrentLocalArmsLodGroup();
        if (armRefreshRequested || (maintenanceDue && armChamsEnabled)) UpdateArmChams(liveArmsLodGroup);
        if (gloveRefreshRequested || (maintenanceDue && gloveChamsEnabled)) UpdateGloveChams(liveArmsLodGroup);
    }

    AnimateWeaponChamsColor();
    AnimateArmChamsColor();
    AnimateGloveChamsColor();
    AnimateWorldColor();
    UpdateAimbotAutoFire();

    if (InterlockedExchange(&pendingScopeOverlayRefresh, 0)) ApplyScopeOverlayState();

    const LONG worldColorCommand = InterlockedExchange(&pendingWorldColorCommand, 0);
    if (worldColorCommand == 2) {
        worldColorNextRetryTick = 0;
        worldColorRetryCount = 0;
        RestoreWorldColor();
    } else if (keyValidated && worldColorEnabled && worldColorCommand == 1) {
        if (!worldColorRenderers.empty()) ApplyWorldColorToCache();
        else { worldColorRetryCount = 0; worldColorNextRetryTick = chamsNow; }
    }

    if (keyValidated && worldColorEnabled && worldColorRenderers.empty() &&
        worldColorNextRetryTick && chamsNow >= worldColorNextRetryTick) {
        ++worldColorRetryCount;
        if (CaptureWorldColorMaterials()) {
            worldColorRetryCount = 0;
            worldColorNextRetryTick = 0;
        } else if (worldColorRetryCount < worldColorMaxRetries) {
            worldColorNextRetryTick = chamsNow + 1000;
            sprintf_s(worldColorStatus, "Map loading; retry %d/%d", worldColorRetryCount, worldColorMaxRetries);
        } else {
            worldColorNextRetryTick = 0;
            strcpy_s(worldColorStatus, "Map not ready; toggle World Color to retry");
        }
    }

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
        
        motion.y = 0;
    }

    if (keyValidated) {
        motion = EdgeBug::ApplyDownwardPull(motion, edgeBugEnabled, edgeBugPullForce);
        // Enforce the configured horizontal units/second cap on the movement
        // passed to Unity, using the real frame delta.
        motion = ApplyVelocityLimitToMotion(motion);
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
    void* localPC = GetLocalPC();
    Vector3 localPosition;
    const bool haveLocalPosition = localPC && GetPCPosition(localPC, localPosition);
    const unsigned char localTeam = GetPCTeam(localPC);

    void* players[64];
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    InterlockedExchange(&boxEspEnumerated, playerCount);
    if (playerCount <= 0 || !o_WorldToScreenPoint) {
        InterlockedExchange(&boxEspProjected, 0);
        InterlockedExchange(&boxEspDrawn, 0);
        return;
    }

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    int projected = 0;
    int drawn = 0;
    for (int i = 0; i < playerCount && drawn < espCount; ++i) {
        void* player = players[i];
        if (!player || player == localPC) continue;
        const unsigned char playerTeam = GetPCTeam(player);
        // Enemy-only ESP. Do not treat unassigned/spectator entries as enemies.
        if (localTeam == 0 || localTeam == 3 || playerTeam == 0 || playerTeam == 3 || playerTeam == localTeam)
            continue;

        Vector3 basePosition;
        if (!GetPCPosition(player, basePosition)) continue;
        if (haveLocalPosition) {
            const float distance = basePosition.Distance(localPosition);
            if (distance < 0.5f || distance > espMaxDistance) continue;
        }

        Vector3 headPosition = basePosition;
        Vector3 feetPosition = basePosition;
        bool haveHead = false;
        bool haveFeet = false;
        __try {
            const uintptr_t biped = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(player) + 0x118);
            if (biped && o_Transform_get_position) {
                const uintptr_t head = *reinterpret_cast<uintptr_t*>(biped + 0x20);
                const uintptr_t leftFoot = *reinterpret_cast<uintptr_t*>(biped + 0xA0);
                const uintptr_t rightFoot = *reinterpret_cast<uintptr_t*>(biped + 0xC0);
                if (head) { headPosition = o_Transform_get_position(head); haveHead = true; }
                if (leftFoot && rightFoot) {
                    const Vector3 left = o_Transform_get_position(leftFoot);
                    const Vector3 right = o_Transform_get_position(rightFoot);
                    feetPosition = Vector3((left.x + right.x) * 0.5f, (left.y + right.y) * 0.5f, (left.z + right.z) * 0.5f);
                    haveFeet = true;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
        if (!haveHead) headPosition.y += 1.75f;
        if (!haveFeet) feetPosition.y -= 0.1f;

        Vector2 headScreen, feetScreen;
        if (!UnityWorldToScreen(headPosition, headScreen) || !UnityWorldToScreen(feetPosition, feetScreen)) continue;
        ++projected;
        float height = fabsf(feetScreen.y - headScreen.y);
        if (height < 8.0f || height > ImGui::GetIO().DisplaySize.y * 1.5f) continue;
        const float width = height * 0.48f;
        const float centerX = (headScreen.x + feetScreen.x) * 0.5f;
        const float top = fminf(headScreen.y, feetScreen.y);
        const float bottom = fmaxf(headScreen.y, feetScreen.y);
        const ImVec2 boxMin(centerX - width * 0.5f, top);
        const ImVec2 boxMax(centerX + width * 0.5f, bottom);
        draw->AddRect(boxMin, boxMax, IM_COL32(0, 0, 0, 220), 2.0f, 0, 4.0f);
        if (espGradient) {
            constexpr int gradientSegments = 32;
            for (int segment = 0; segment < gradientSegments; ++segment) {
                const float t0 = static_cast<float>(segment) / gradientSegments;
                const float t1 = static_cast<float>(segment + 1) / gradientSegments;
                const float y0 = top + height * t0;
                const float y1 = top + height * t1;
                const ImU32 color = EspColorAt((t0 + t1) * 0.5f);
                draw->AddLine(ImVec2(boxMin.x, y0), ImVec2(boxMin.x, y1), color, 2.0f);
                draw->AddLine(ImVec2(boxMax.x, y0), ImVec2(boxMax.x, y1), color, 2.0f);
            }
            draw->AddLine(boxMin, ImVec2(boxMax.x, boxMin.y), EspColorAt(0.0f), 2.0f);
            draw->AddLine(ImVec2(boxMin.x, boxMax.y), boxMax, EspColorAt(1.0f), 2.0f);
        } else {
            draw->AddRect(boxMin, boxMax, EspColorAt(0.0f), 2.0f, 0, 1.7f);
        }

        if (espShowName) {
            char playerName[513];
            GetPCName(player, playerName, sizeof(playerName));
            const ImVec2 size = ImGui::CalcTextSize(playerName);
            const ImVec2 pos(centerX - size.x * 0.5f, top - size.y - 3.0f);
            draw->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), playerName);
            draw->AddText(pos, EspArrayColor(espNameColor), playerName);
        }

        if (espShowHealth) {
            const int health = GetPCHealth(player);
            if (health >= 0) {
                const float healthFraction = fmaxf(0.0f, fminf(1.0f, health / 100.0f));
                const float barX = boxMin.x - 9.0f;
                draw->AddRectFilled(ImVec2(barX - 1.0f, top - 1.0f), ImVec2(barX + 6.0f, bottom + 1.0f), IM_COL32(0, 0, 0, 235));
                const float fillTop = bottom - height * healthFraction;
                constexpr int healthSegments = 24;
                for (int segment = 0; segment < healthSegments; ++segment) {
                    const float t0 = static_cast<float>(segment) / healthSegments;
                    const float t1 = static_cast<float>(segment + 1) / healthSegments;
                    const float segmentBottom = bottom - (bottom - fillTop) * t0;
                    const float segmentTop = bottom - (bottom - fillTop) * t1;
                    draw->AddRectFilled(ImVec2(barX, segmentTop), ImVec2(barX + 5.0f, segmentBottom),
                        EspHealthColorAt(1.0f - (t0 + t1) * 0.5f));
                }
                char hpText[16]; sprintf_s(hpText, "%d HP", health);
                const ImVec2 hpPos(boxMax.x + 5.0f, top);
                DrawAsciiGradientText(draw, hpPos, hpText);
            }
        }

        float infoY = bottom + 3.0f;
        if (espShowWeapon) {
            const char* weaponName = GetPCWeaponName(player);
            const ImVec2 size = ImGui::CalcTextSize(weaponName);
            const ImVec2 pos(centerX - size.x * 0.5f, infoY);
            draw->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), weaponName);
            draw->AddText(pos, EspColorAt(1.0f), weaponName);
            infoY += size.y + 1.0f;
        }
        if (haveLocalPosition) {
            char distanceText[32]; sprintf_s(distanceText, "%.0fm", basePosition.Distance(localPosition));
            const ImVec2 size = ImGui::CalcTextSize(distanceText);
            const ImVec2 pos(centerX - size.x * 0.5f, infoY);
            draw->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), distanceText);
            draw->AddText(pos, IM_COL32(235, 235, 235, 255), distanceText);
        }
        ++drawn;
    }
    InterlockedExchange(&boxEspProjected, projected);
    InterlockedExchange(&boxEspDrawn, drawn);
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
            ImGui::SliderFloat("Max Horizontal Speed", &velocityLimit, 0.01f, 1000.0f, "%.2f units/s");
            ImGui::SetNextItemWidth(160.0f);
            if (ImGui::InputFloat("Exact Speed", &velocityLimit, 0.01f, 1.0f, "%.3f")) {
                if (!isfinite(velocityLimit)) velocityLimit = 1.0f;
                if (velocityLimit < 0.001f) velocityLimit = 0.001f;
                if (velocityLimit > 10000.0f) velocityLimit = 10000.0f;
            }
            ImGui::Text("Current: %.3f units/s", velocityLimiterCurrentHorizontalSpeed);
            ImGui::Text("Limiter: %s", velocityLimiterLastAppliedScale < 0.999f ? "CLAMPING" : "within limit");
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
        ImGui::TextColored(accent, "Aimbot");
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Checkbox("Silent 360 Aimbot", &aimbotEnabled)) {
            InterlockedExchange(&aimbotShots, 0);
            InterlockedExchange(&aimbotTargetsScanned, 0);
            InterlockedExchange(&aimbotVisibleTargets, 0);
            InterlockedExchange(&aimbotApplied, 0);
            strcpy_s(aimbotStatus, (aimbotEnabled || visibleAimbotEnabled) ? "Enabled; waiting for local shot" : "Disabled");
        }
        if (ImGui::Checkbox("Visible Camera Snap", &visibleAimbotEnabled)) {
            if (!visibleAimbotEnabled) CancelVisibleAimbotCamera();
            strcpy_s(aimbotStatus, (aimbotEnabled || visibleAimbotEnabled) ? "Enabled; waiting for local shot" : "Disabled");
        }
        if (visibleAimbotEnabled)
            ImGui::SliderFloat("Camera Hold", &visibleAimbotHoldMs, 50.0f, 400.0f, "%.0f ms");
        ImGui::Checkbox("Visible Check", &aimbotVisibleCheck);
        ImGui::Checkbox("Auto Fire", &aimbotAutoFire);
        ImGui::Text("FOV: %.0f degrees", aimbotFov);
        ImGui::TextWrapped("Status: %s", aimbotStatus);
        if (aimbotEnabled || visibleAimbotEnabled)
            ImGui::Text("Shots: %ld | scanned: %ld | visible: %ld | applied: %ld",
                InterlockedCompareExchange(&aimbotShots, 0, 0),
                InterlockedCompareExchange(&aimbotTargetsScanned, 0, 0),
                InterlockedCompareExchange(&aimbotVisibleTargets, 0, 0),
                InterlockedCompareExchange(&aimbotApplied, 0, 0));
        if (aimbotAutoFire)
            ImGui::Text("Auto-fired: %ld | visibility scans: %ld (20/s max)",
                InterlockedCompareExchange(&aimbotAutoFired, 0, 0),
                InterlockedCompareExchange(&aimbotVisibilityScans, 0, 0));
        ImGui::Spacing();
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
        if (ImGui::Checkbox("Silent Backward + Down", &silentAntiAimEnabled)) {
            InterlockedExchange(&silentAntiAimInputBuildCalls, 0);
            InterlockedExchange(&silentAntiAimCommandCalls, 0);
            InterlockedExchange(&silentAntiAimAppliedCalls, 0);
            InterlockedExchange(&silentAntiAimSnapshotCalls, 0);
            silentAntiAimLatestInputValid = false;
            silentAntiAimLatestFiring = false;
            silentAntiAimLatestPlayer = 0;
            strcpy_s(silentAntiAimStatus, !silentAntiAimEnabled ? "Disabled" :
                (silentAntiAimHookReady ? "Input + detached snapshot hooks installed" : "Anti-aim hooks failed to install"));
        }
        ImGui::TextWrapped("Status: %s", silentAntiAimStatus);
        if (silentAntiAimEnabled)
            ImGui::Text("Input: %ld | Snapshots: %ld | Applied: %ld | qhq: %ld",
                InterlockedCompareExchange(&silentAntiAimInputBuildCalls, 0, 0),
                InterlockedCompareExchange(&silentAntiAimSnapshotCalls, 0, 0),
                InterlockedCompareExchange(&silentAntiAimAppliedCalls, 0, 0),
                InterlockedCompareExchange(&silentAntiAimCommandCalls, 0, 0));
        ImGui::Spacing();
        ImGui::TextWrapped("Live aim, camera, local model and movement stay untouched. Fake angles exist only in the detached AimSnapshot used by snapshot consumers.");
        ImGui::EndChild();
    }
    else if (currentTab == 2) { // VISUALS
        ImGui::BeginChild("Visuals", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Visuals");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Checkbox("Box ESP", &boxEsp);
        if (boxEsp) {
            ImGui::SliderInt("ESP Player Limit", &espCount, 1, 64);
            ImGui::SliderFloat("ESP Max Distance", &espMaxDistance, 10.0f, 500.0f, "%.0f m");
            ImGui::Checkbox("ESP Names", &espShowName);
            ImGui::Checkbox("ESP Health", &espShowHealth);
            ImGui::Checkbox("ESP Weapons", &espShowWeapon);
            ImGui::Checkbox("ESP Gradient", &espGradient);
            ImGui::ColorEdit3("ESP Box Color", espTopColor);
            if (espGradient) ImGui::ColorEdit3("ESP Box Gradient Color", espBottomColor);
            ImGui::ColorEdit3("ESP Name Color", espNameColor);
            ImGui::ColorEdit3("ESP Health Color", espHealthColor);
            ImGui::Checkbox("ESP Health Gradient", &espHealthGradient);
            if (espHealthGradient) ImGui::ColorEdit3("ESP Health Gradient Color", espHealthBottomColor);
            ImGui::Text("ESP players: %ld | projected: %ld | drawn: %ld | HP updates: %ld",
                InterlockedCompareExchange(&boxEspEnumerated, 0, 0),
                InterlockedCompareExchange(&boxEspProjected, 0, 0),
                InterlockedCompareExchange(&boxEspDrawn, 0, 0),
                InterlockedCompareExchange(&boxEspHealthUpdates, 0, 0));
            ImGui::Text("HP matched: %ld | last args: %ld/%ld | getter: %ld",
                InterlockedCompareExchange(&boxEspHealthPlayerMatches, 0, 0),
                InterlockedCompareExchange(&boxEspHealthLastA, 0, 0),
                InterlockedCompareExchange(&boxEspHealthLastB, 0, 0),
                InterlockedCompareExchange(&boxEspHealthLastRead, 0, 0));
            ImGui::Text("Confirmed hits: %ld | HP: %ld | result: %ld/%ld/%ld",
                InterlockedCompareExchange(&boxEspConfirmedHits, 0, 0),
                InterlockedCompareExchange(&boxEspConfirmedHealth, 0, 0),
                InterlockedCompareExchange(&boxEspConfirmedFieldA, 0, 0),
                InterlockedCompareExchange(&boxEspConfirmedFieldB, 0, 0),
                InterlockedCompareExchange(&boxEspConfirmedFieldC, 0, 0));
            ImGui::Text("Sources - dict: %ld | unity: %ld | hooks: %ld",
                InterlockedCompareExchange(&boxEspDictionaryCount, 0, 0),
                InterlockedCompareExchange(&boxEspUnityCount, 0, 0),
                InterlockedCompareExchange(&boxEspHookCount, 0, 0));
        }
        ImGui::Checkbox("Show Velocity", &showVelocity);
        ImGui::Checkbox("Show Trail", &showTrail);
        if (ImGui::Checkbox("World Color", &worldColorEnabled))
            InterlockedExchange(&pendingWorldColorCommand, worldColorEnabled ? 1 : 2);
        if (worldColorEnabled) {
            const char* worldModes[] = { "Textured Tint", "Flat", "Glass", "Lit", "Metallic", "Rainbow", "Pulse" };
            if (ImGui::Combo("World Mode", &worldColorMode, worldModes, IM_ARRAYSIZE(worldModes)))
                InterlockedExchange(&pendingWorldColorCommand, 1);
            if (worldColorMode != 5 && ImGui::ColorEdit3("Map Material Color", worldColor))
                InterlockedExchange(&pendingWorldColorCommand, 1);
            if (worldColorMode == 0 && ImGui::SliderFloat("Tint Strength", &worldColorStrength, 0.0f, 1.0f, "%.2f"))
                InterlockedExchange(&pendingWorldColorCommand, 1);
            if (worldColorMode == 2 && ImGui::SliderFloat("Glass Alpha", &worldColorAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingWorldColorCommand, 1);
            if (worldColorMode == 4) {
                if (ImGui::SliderFloat("World Metallic", &worldColorMetallic, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingWorldColorCommand, 1);
                if (ImGui::SliderFloat("World Smoothness", &worldColorSmoothness, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingWorldColorCommand, 1);
            }
            if ((worldColorMode == 5 || worldColorMode == 6) &&
                ImGui::SliderFloat("World Animation Speed", &worldColorAnimationSpeed, 0.1f, 5.0f, "%.2f"))
                InterlockedExchange(&pendingWorldColorCommand, 1);
            ImGui::TextWrapped("World status: %s", worldColorStatus);
        }
        if (ImGui::Checkbox("Third Person", &thirdPersonEnabled)) {
            strcpy_s(thirdPersonStatus, thirdPersonEnabled ? "TPS transition queued" : "FPS transition queued");
            InterlockedExchange(&pendingThirdPersonCommand, 1);
        }
        if (thirdPersonEnabled) {
            ImGui::TextWrapped("Third person status: %s", thirdPersonStatus);
            ImGui::SliderFloat("TPS Horizontal", &thirdPersonHorizontalOffset, -3.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("TPS Height", &thirdPersonHeightAdjustment, -3.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("TPS Distance", &thirdPersonDistanceAdjustment, -6.0f, 6.0f, "%.2f");
            if (ImGui::Button("Reset TPS Camera")) {
                thirdPersonHorizontalOffset = 0.0f;
                thirdPersonHeightAdjustment = 0.0f;
                thirdPersonDistanceAdjustment = 0.0f;
            }
        }
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
        
        if (ImGui::Checkbox("Infinity Ammo", &infinityAmmo)) {
            frozenAmmoWeapon = 0;
            frozenAmmoValue = -1;
            InterlockedExchange(&infinityAmmoFireCalls, 0);
            InterlockedExchange(&infinityAmmoGetterCalls, 0);
            InterlockedExchange(&infinityAmmoRestores, 0);
        }
        if (infinityAmmo)
            ImGui::Text("Magazine freeze: %d | cegy/wyy: %ld/%ld | fire/get/restore: %ld/%ld/%ld",
                static_cast<int>(frozenAmmoValue),
                InterlockedCompareExchange(&infinityAmmoLastField, 0, 0),
                InterlockedCompareExchange(&infinityAmmoLastGetter, 0, 0),
                InterlockedCompareExchange(&infinityAmmoFireCalls, 0, 0),
                InterlockedCompareExchange(&infinityAmmoGetterCalls, 0, 0),
                InterlockedCompareExchange(&infinityAmmoRestores, 0, 0));
        ImGui::Checkbox("No Spread", &noSpreadEnabled);
        if (ImGui::Checkbox("Remove Scope Borders", &removeScopeBorders)) InterlockedExchange(&pendingScopeOverlayRefresh, 1);
        if (ImGui::Checkbox("Weapon Chams", &weaponChamsEnabled)) {
            strcpy_s(weaponChamsStatus, weaponChamsEnabled ? "Applying to equipped weapon" : "Restoring original materials");
            InterlockedExchange(&pendingWeaponChamsRefresh, 1);
        }
        if (weaponChamsEnabled) {
            const char* chamsModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse" };
            if (ImGui::Combo("Chams Material", &weaponChamsMode, chamsModes, IM_ARRAYSIZE(chamsModes))) {
                weaponChamsController = 0;
                InterlockedExchange(&pendingWeaponChamsRefresh, 1);
            }
            if (ImGui::ColorEdit3("Chams Color", weaponChamsColor)) {
                InterlockedExchange(&pendingWeaponChamsRefresh, 1);
            }
            if ((weaponChamsMode == 1 || weaponChamsMode == 4) && ImGui::SliderFloat("Transparency", &weaponChamsGlassAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingWeaponChamsRefresh, 1);
            if (weaponChamsMode == 3) {
                if (ImGui::SliderFloat("Metallic", &weaponChamsMetallic, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingWeaponChamsRefresh, 1);
                if (ImGui::SliderFloat("Smoothness", &weaponChamsSmoothness, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingWeaponChamsRefresh, 1);
            }
            if (weaponChamsMode == 5 || weaponChamsMode == 6)
                ImGui::SliderFloat("Animation Speed", &weaponChamsAnimationSpeed, 0.2f, 5.0f, "%.1f");
            ImGui::TextWrapped("Weapon status: %s", weaponChamsStatus);
        }
        if (ImGui::Checkbox("Arm Chams", &armChamsEnabled)) {
            strcpy_s(armChamsStatus, armChamsEnabled ? "Applying to arms" : "Restoring original arm material");
            InterlockedExchange(&pendingArmChamsRefresh, 1);
        }
        if (armChamsEnabled) {
            const char* handModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse" };
            if (ImGui::Combo("Arm Material", &armChamsMode, handModes, IM_ARRAYSIZE(handModes))) {
                // Keep the captured ArmsLodGroup and original materials. The game-thread
                // refresh below swaps only the replacement material on the same renderers.
                strcpy_s(armChamsStatus, "Switching arm material");
                InterlockedExchange(&pendingArmChamsRefresh, 1);
            }
            if (ImGui::ColorEdit3("Arm Color", armChamsColor)) InterlockedExchange(&pendingArmChamsRefresh, 1);
            if ((armChamsMode == 1 || armChamsMode == 4) && ImGui::SliderFloat("Arm Transparency", &armChamsAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingArmChamsRefresh, 1);
            if (armChamsMode == 3) {
                if (ImGui::SliderFloat("Arm Metallic", &armChamsMetallic, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingArmChamsRefresh, 1);
                if (ImGui::SliderFloat("Arm Smoothness", &armChamsSmoothness, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingArmChamsRefresh, 1);
            }
            if (armChamsMode == 5 || armChamsMode == 6)
                ImGui::SliderFloat("Arm Animation Speed", &armChamsAnimationSpeed, 0.2f, 5.0f, "%.1f");
            ImGui::TextWrapped("Arm status: %s", armChamsStatus);
        }
        if (ImGui::Checkbox("Glove Chams", &gloveChamsEnabled)) {
            strcpy_s(gloveChamsStatus, gloveChamsEnabled ? "Applying to gloves" : "Restoring original glove material");
            InterlockedExchange(&pendingGloveChamsRefresh, 1);
        }
        if (gloveChamsEnabled) {
            const char* gloveModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse" };
            if (ImGui::Combo("Glove Material", &gloveChamsMode, gloveModes, IM_ARRAYSIZE(gloveModes))) {
                strcpy_s(gloveChamsStatus, "Switching glove material");
                InterlockedExchange(&pendingGloveChamsRefresh, 1);
            }
            if (ImGui::ColorEdit3("Glove Color", gloveChamsColor)) InterlockedExchange(&pendingGloveChamsRefresh, 1);
            if ((gloveChamsMode == 1 || gloveChamsMode == 4) && ImGui::SliderFloat("Glove Transparency", &gloveChamsAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingGloveChamsRefresh, 1);
            if (gloveChamsMode == 3) {
                if (ImGui::SliderFloat("Glove Metallic", &gloveChamsMetallic, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingGloveChamsRefresh, 1);
                if (ImGui::SliderFloat("Glove Smoothness", &gloveChamsSmoothness, 0.0f, 1.0f, "%.2f")) InterlockedExchange(&pendingGloveChamsRefresh, 1);
            }
            if (gloveChamsMode == 5 || gloveChamsMode == 6)
                ImGui::SliderFloat("Glove Animation Speed", &gloveChamsAnimationSpeed, 0.2f, 5.0f, "%.1f");
            ImGui::TextWrapped("Glove status: %s", gloveChamsStatus);
        }
        ImGui::Checkbox("Hit Marker", &hitMarkerEnabled);
        if (hitMarkerEnabled) {
            ImGui::ColorEdit3("Hit Marker Color", hitMarkerColor);
            ImGui::SliderFloat("Hit Marker Duration", &hitMarkerDuration, 0.08f, 1.0f, "%.2f s");
            ImGui::SliderFloat("Hit Marker Size", &hitMarkerSize, 3.0f, 24.0f, "%.0f px");
            ImGui::SliderFloat("Hit Marker Gap", &hitMarkerGap, 0.0f, 16.0f, "%.0f px");
            ImGui::SliderFloat("Hit Marker Thickness", &hitMarkerThickness, 1.0f, 6.0f, "%.1f px");
            int activeMarkerCount = 0;
            AcquireSRWLockShared(&hitMarkerLock);
            activeMarkerCount = static_cast<int>(hitMarkers.size());
            ReleaseSRWLockShared(&hitMarkerLock);
            ImGui::Text("Confirmed: %ld | Active: %d | Resolved: %ld | Fallback: %ld",
                InterlockedCompareExchange(&hitMarkerCalls, 0, 0), activeMarkerCount,
                InterlockedCompareExchange(&hitMarkerResolvedCalls, 0, 0),
                InterlockedCompareExchange(&hitMarkerFallbackCalls, 0, 0));
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

    DrawHitMarker();
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
        g_GameControllerClass = g_il2cpp.find_class("Axlebolt.Standoff.Game", "GameController");
        if (g_GameControllerClass)
            g_GameControllerInstanceField = g_il2cpp.class_get_field_from_name(g_GameControllerClass, "<ceva>k__BackingField");
        g_PlayerManagerClass = g_il2cpp.find_class("Axlebolt.Standoff.Player", "PlayerManager");
        if (!g_PlayerManagerClass) g_PlayerManagerClass = g_il2cpp.find_class("", "PlayerManager");
        LINDY_LOG("[init] PlayerManagerClass=%p", (void*)g_PlayerManagerClass);
        if (g_PlayerManagerClass) {
            Il2CppClass* playerControllerClass = g_il2cpp.find_class("Axlebolt.Standoff.Player", "PlayerController");
            if (!playerControllerClass) playerControllerClass = g_il2cpp.find_class("", "PlayerController");
            if (playerControllerClass && g_il2cpp.class_get_type && g_il2cpp.type_get_object) {
                const Il2CppType* playerType = g_il2cpp.class_get_type(playerControllerClass);
                g_PlayerControllerReflectionType = playerType ? g_il2cpp.type_get_object(playerType) : nullptr;
            }
            LINDY_LOG("[init] PlayerController reflection type=%p", (void*)g_PlayerControllerReflectionType);
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
        g_GlovesManagerClass = g_il2cpp.find_class("Axlebolt.Standoff.Main.Inventory.Gloves", "GlovesManager");
        LINDY_LOG("[init] GlovesManagerClass=%p", (void*)g_GlovesManagerClass);
        if (g_GlovesManagerClass) {
            // Singleton<GlovesManager> stores the active instance in parent field cgyg.
            Il2CppClass* parent = g_il2cpp.class_get_parent ? g_il2cpp.class_get_parent(g_GlovesManagerClass) : nullptr;
            if (parent) g_GlovesManagerInstanceField = g_il2cpp.class_get_field_from_name(parent, "cgyg");
            if (!g_GlovesManagerInstanceField)
                g_GlovesManagerInstanceField = g_il2cpp.class_get_field_from_name(g_GlovesManagerClass, "cgyg");
        }
    }
    base = (uintptr_t)GetModuleHandleA("GameAssembly.dll");

    unityPlayerBase = (uintptr_t)GetModuleHandleA("UnityPlayer.dll");

    if (!base || !unityPlayerBase) return 0;

    

    g_MatrixValid = true;



    o_CheatRuntime_SetThirdPerson = (void(__fastcall*)(uintptr_t, bool))(base + OFFSET_CHEAT_RUNTIME_SET_THIRDPERSON);
    o_GetPlayerController = (uintptr_t(__fastcall*)())(base + OFFSET_GET_PLAYERCONTROLLER);

    const MH_STATUS playerEventACreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_A),
        hk_PlayerManagerPlayerEventA, (LPVOID*)&o_PlayerManagerPlayerEventA);
    const MH_STATUS playerEventAEnable = playerEventACreate == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_A)) : playerEventACreate;
    const MH_STATUS playerEventBCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_B),
        hk_PlayerManagerPlayerEventB, (LPVOID*)&o_PlayerManagerPlayerEventB);
    const MH_STATUS playerEventBEnable = playerEventBCreate == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_B)) : playerEventBCreate;
    const MH_STATUS playerEventCCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_C),
        hk_PlayerManagerPlayerEventC, (LPVOID*)&o_PlayerManagerPlayerEventC);
    const MH_STATUS playerEventCEnable = playerEventCCreate == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_PLAYERMANAGER_PLAYER_EVENT_C)) : playerEventCCreate;
    LINDY_LOG("[BoxEsp] player event hooks A=%d/%d B=%d/%d C=%d/%d",
        (int)playerEventACreate, (int)playerEventAEnable, (int)playerEventBCreate,
        (int)playerEventBEnable, (int)playerEventCCreate, (int)playerEventCEnable);

    const MH_STATUS healthACreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_A), hk_HealthApplyA, (LPVOID*)&o_HealthApplyA);
    if (healthACreate == MH_OK || healthACreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_A));
    const MH_STATUS healthBCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_B), hk_HealthApplyB, (LPVOID*)&o_HealthApplyB);
    if (healthBCreate == MH_OK || healthBCreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_B));
    const MH_STATUS healthCCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_C), hk_HealthApplyC, (LPVOID*)&o_HealthApplyC);
    if (healthCCreate == MH_OK || healthCCreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_C));
    const MH_STATUS healthDCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_D), hk_HealthApplyD, (LPVOID*)&o_HealthApplyD);
    if (healthDCreate == MH_OK || healthDCreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HEALTH_APPLY_D));

    const MH_STATUS hitConfirmedACreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HIT_CONFIRMED_A), hk_PlayerHitConfirmedA, (LPVOID*)&o_PlayerHitConfirmedA);
    if (hitConfirmedACreate == MH_OK || hitConfirmedACreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HIT_CONFIRMED_A));
    const MH_STATUS hitConfirmedBCreate = MH_CreateHook((LPVOID)(base + OFFSET_PLAYER_HIT_CONFIRMED_B), hk_PlayerHitConfirmedB, (LPVOID*)&o_PlayerHitConfirmedB);
    if (hitConfirmedBCreate == MH_OK || hitConfirmedBCreate == MH_ERROR_ALREADY_CREATED) MH_EnableHook((LPVOID)(base + OFFSET_PLAYER_HIT_CONFIRMED_B));

    o_Transform_get_position = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_POSITION);
    o_Transform_get_forward = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_FORWARD);
    o_Transform_get_eulerAngles = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_EULERANGLES);
    o_Transform_set_eulerAngles = (void(__fastcall*)(uintptr_t, Vector3))(base + OFFSET_TRANSFORM_SET_EULERANGLES);

    o_Component_get_transform = (uintptr_t(__fastcall*)(uintptr_t))(base + OFFSET_COMPONENT_GET_TRANSFORM);

    o_Camera_get_main = (uintptr_t(__fastcall*)())(base + OFFSET_CAMERA_MAIN);
    o_Time_get_deltaTime = (float(__fastcall*)())(base + OFFSET_TIME_GET_DELTATIME);

    MH_CreateHook((LPVOID)(base + OFFSET_CAMERA_SET_FIELDOFVIEW), hk_Camera_set_fieldOfView, (LPVOID*)&o_Camera_set_fieldOfView);

    MH_EnableHook((LPVOID)(base + OFFSET_CAMERA_SET_FIELDOFVIEW));

    o_Camera_set_backgroundColor = (void(__fastcall*)(uintptr_t, Color))(base + OFFSET_CAMERA_SET_BACKGROUNDCOLOR);

    o_Camera_set_clearFlags = (void(__fastcall*)(uintptr_t, int))(base + OFFSET_CAMERA_SET_CLEARFLAGS);

    o_Texture2D_ctor = (void(__fastcall*)(uintptr_t, int, int, const Il2CppMethod*))(base + OFFSET_TEXTURE2D_CTOR);
    o_ImageConversion_LoadImage = (bool(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_IMAGECONVERSION_LOADIMAGE);
    o_Shader_Find = (uintptr_t(__fastcall*)(Il2CppString*, const Il2CppMethod*))(base + OFFSET_SHADER_FIND);
    o_Material_ctor = (void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_MATERIAL_CTOR);
    o_Material_copy_ctor = (void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_MATERIAL_COPY_CTOR);
    o_Material_set_shader = (void(__fastcall*)(uintptr_t, uintptr_t))(base + OFFSET_MATERIAL_SET_SHADER);
    o_Material_set_mainTexture = (void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*))(base + OFFSET_MATERIAL_SET_MAINTEXTURE);
    o_Renderer_get_material = (uintptr_t(__fastcall*)(uintptr_t))(base + OFFSET_RENDERER_GET_MATERIAL);
    o_Renderer_set_material = (void(__fastcall*)(uintptr_t, uintptr_t))(base + OFFSET_RENDERER_SET_MATERIAL);
    o_Renderer_get_materials = (Il2CppArray*(__fastcall*)(uintptr_t))(base + OFFSET_RENDERER_GET_MATERIALS);
    o_Renderer_get_isPartOfStaticBatch = (bool(__fastcall*)(uintptr_t))(base + OFFSET_RENDERER_IS_STATIC_BATCH);
    o_Renderer_set_materials = (void(__fastcall*)(uintptr_t, Il2CppArray*))(base + OFFSET_RENDERER_SET_MATERIALS);
    o_Object_FindObjectsOfType = (Il2CppArray*(__fastcall*)(Il2CppObject*, bool, const Il2CppMethod*))(base + OFFSET_OBJECT_FIND_OBJECTS_OF_TYPE);
    o_Material_get_color = (Color(__fastcall*)(uintptr_t))(base + OFFSET_MATERIAL_GET_COLOR);
    o_Material_set_color = (void(__fastcall*)(uintptr_t, Color))(base + OFFSET_MATERIAL_SET_COLOR);
    o_Material_set_renderQueue = (void(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_SET_RENDERQUEUE);
    o_Material_SetFloat = (void(__fastcall*)(uintptr_t, Il2CppString*, float))(base + OFFSET_MATERIAL_SET_FLOAT);
    o_Material_HasProperty = (bool(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_HAS_PROPERTY);
    o_Material_GetColorId = (Color(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_GET_COLOR_ID);
    o_Material_SetColorId = (void(__fastcall*)(uintptr_t, int, Color))(base + OFFSET_MATERIAL_SET_COLOR_ID);
    o_Shader_PropertyToID = (int(__fastcall*)(Il2CppString*, const Il2CppMethod*))(base + OFFSET_SHADER_PROPERTY_TO_ID);
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
    const MH_STATUS hitMarkerCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_HITMARKERVIEW_SHOW), hk_HitMarkerView_Show,
        (LPVOID*)&o_HitMarkerView_Show);
    const MH_STATUS hitMarkerEnableStatus = hitMarkerCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_HITMARKERVIEW_SHOW)) : hitMarkerCreateStatus;
    if (hitMarkerCreateStatus != MH_OK || hitMarkerEnableStatus != MH_OK)
        hitMarkerEnabled = false;
    o_AimingData_Clone = (uintptr_t(__fastcall*)(uintptr_t, const Il2CppMethod*))
        (base + OFFSET_AIMINGDATA_CLONE);
    const MH_STATUS antiAimSnapshotCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_AIMCONTROLLER_GET_SNAPSHOT),
        hk_AimController_GetSnapshot,
        (LPVOID*)&o_AimController_GetSnapshot);
    const MH_STATUS antiAimSnapshotEnableStatus =
        antiAimSnapshotCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_AIMCONTROLLER_GET_SNAPSHOT)) :
        antiAimSnapshotCreateStatus;

    const MH_STATUS antiAimInputCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_KEYBOARDCONTROL_BUILD_COMMAND),
        hk_KeyboardControl_BuildCommand,
        (LPVOID*)&o_KeyboardControl_BuildCommand);
    const MH_STATUS antiAimInputEnableStatus = antiAimInputCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_KEYBOARDCONTROL_BUILD_COMMAND)) :
        antiAimInputCreateStatus;
    silentAntiAimHookReady = antiAimSnapshotCreateStatus == MH_OK &&
        antiAimSnapshotEnableStatus == MH_OK &&
        antiAimInputCreateStatus == MH_OK &&
        antiAimInputEnableStatus == MH_OK;

    const MH_STATUS antiAimFallbackCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_PLAYERCONTROLLER_COMMAND),
        hk_PlayerController_Command,
        (LPVOID*)&o_PlayerController_Command);
    const MH_STATUS antiAimFallbackEnableStatus =
        antiAimFallbackCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_PLAYERCONTROLLER_COMMAND)) :
        antiAimFallbackCreateStatus;
    LINDY_LOG("[anti-aim] AimController.qdr create=%d enable=%d; KeyboardControl.bbft create=%d enable=%d ready=%d; qhq diagnostic create=%d enable=%d",
        (int)antiAimSnapshotCreateStatus, (int)antiAimSnapshotEnableStatus,
        (int)antiAimInputCreateStatus, (int)antiAimInputEnableStatus,
        silentAntiAimHookReady ? 1 : 0,
        (int)antiAimFallbackCreateStatus, (int)antiAimFallbackEnableStatus);

    // Capture the live ArmsLodGroup directly whenever its first-person visibility is enabled.
    MH_CreateHook((LPVOID)(base + OFFSET_ARMSLOD_SET_VISIBLE), hk_ArmsLod_SetVisible, (LPVOID*)&o_ArmsLod_SetVisible);
    MH_EnableHook((LPVOID)(base + OFFSET_ARMSLOD_SET_VISIBLE));

    // Reapply arm/glove chams when the local glove model changes.
    MH_CreateHook((LPVOID)(base + OFFSET_GLOVES_SET_ARMS), hk_Gloves_SetArms, (LPVOID*)&o_Gloves_SetArms);
    MH_EnableHook((LPVOID)(base + OFFSET_GLOVES_SET_ARMS));

    // Actual equip transition for every slot, including knives.
    MH_CreateHook((LPVOID)(base + OFFSET_WEAPONRY_TAKE_WEAPON), hk_Weaponry_TakeWeapon, (LPVOID*)&o_Weaponry_TakeWeapon);
    MH_EnableHook((LPVOID)(base + OFFSET_WEAPONRY_TAKE_WEAPON));

    // Fire remains a fallback for guns injected after the equip event.
    MH_CreateHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE), hk_GunController_Fire, (LPVOID*)&o_GunController_Fire);
    MH_EnableHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE));
    MH_CreateHook((LPVOID)(base + OFFSET_HITCASTER_CAST), hk_HitCaster_Cast, (LPVOID*)&o_HitCaster_Cast);
    MH_EnableHook((LPVOID)(base + OFFSET_HITCASTER_CAST));

    // Getter remains as a secondary HUD/state refresh path.
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
