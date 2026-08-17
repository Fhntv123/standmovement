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
#include <shlobj.h>

#pragma comment(lib, "urlmon.lib")

#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shell32.lib")



extern "C" IMAGE_DOS_HEADER __ImageBase;
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
float menuOpenAnimation = 1.0f;

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



struct RaycastHitNative {
    Vector3 point;
    Vector3 normal;
    uint32_t faceID;
    float distance;
    Vector2 uv;
    int colliderInstanceID;
};
static_assert(sizeof(RaycastHitNative) == 0x2C, "Unity RaycastHit layout changed");


// dump1 exact value-type layouts used by HitCaster.vxe<ceq>.
struct HitCasterCeq {
    uintptr_t damageDefinition; // cdf* +0x00
    float valueA;               // +0x08
    float valueB;               // +0x0C
    int valueC;                 // +0x10
    int padding;                // +0x14
};
static_assert(sizeof(HitCasterCeq) == 0x18, "dump1 ceq layout changed");

struct HitCasterCer {
    Vector3 start;              // +0x00
    Vector3 end;                // +0x0C
    uintptr_t hitDictionary;    // Dictionary<bal,ccr>* +0x18
    uintptr_t penetrations;     // List<cew>* +0x20
    float value;                // +0x28
    int padding;                // +0x2C
};
static_assert(sizeof(HitCasterCer) == 0x30, "dump1 cer layout changed");

struct Color {

    float r, g, b, a;

    Color() : r(0), g(0), b(0), a(1) {}

    Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}

};



struct Matrix16 {

    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;

};



// CharacterController methods

#define OFFSET_CHARACTERCONTROLLER_GET_ISGROUNDED  0x34CEF60

#define OFFSET_CHARACTERCONTROLLER_MOVE            0x34CEE30

#define OFFSET_CHARACTERCONTROLLER_GET_VELOCITY    0x34CF030
#define OFFSET_TIME_GET_DELTATIME                   0x3489450

#define OFFSET_TRANSFORM_GET_POSITION              0x348B9F0
#define OFFSET_TRANSFORM_GET_FORWARD               0x348B490

#define OFFSET_COMPONENT_GET_TRANSFORM             0x34747D0

#define OFFSET_GET_PLAYERCONTROLLER                0xBEA600  // PlayerManager.ncm() -> current player controller

#define OFFSET_CAMERA_MAIN                         0x3442EC0

#define OFFSET_CAMERA_SET_FIELDOFVIEW              0x34438B0

#define OFFSET_CAMERA_SET_BACKGROUNDCOLOR          0x34436C0

#define OFFSET_CAMERA_SET_CLEARFLAGS               0x3443750

#define OFFSET_TEXTURE2D_CTOR                      0x345D030
#define OFFSET_IMAGECONVERSION_LOADIMAGE           0x34ACD00
#define OFFSET_SHADER_FIND                         0x345B670
#define OFFSET_MATERIAL_CTOR                       0x34506B0
#define OFFSET_MATERIAL_COPY_CTOR                  0x34507A0
#define OFFSET_MATERIAL_SET_SHADER                 0x34510A0
#define OFFSET_MATERIAL_SET_MAINTEXTURE            0x3450F90
#define OFFSET_RENDERER_GET_MATERIAL                0x345A740
#define OFFSET_RENDERER_SET_MATERIAL                0x345AAD0
#define OFFSET_RENDERER_GET_MATERIALS                0x345A6C0
#define OFFSET_RENDERER_IS_STATIC_BATCH              0x345AC10
#define OFFSET_RENDERER_SET_MATERIALS                0x345AA60
#define OFFSET_RENDERER_GET_ENABLED                  0x345ABD0
#define OFFSET_RENDERER_SET_ENABLED                  0x345AE00
#define OFFSET_SKINNEDMESHRENDERER_SET_UPDATE_WHEN_OFFSCREEN 0x345BB00
#define OFFSET_OBJECT_OCCLUDEE_SET_VISIBLE_STATE    0xC26260
#define OFFSET_OBJECT_FIND_OBJECTS_OF_TYPE            0x347B5D0
#define OFFSET_OBJECT_OP_IMPLICIT                     0x347C930
#define OFFSET_PHYSICS_RAYCAST_HIT                    0x34D70B0
#define OFFSET_PHYSICS_RAYCAST_ALL                    0x34D5130
#define OFFSET_RAYCASTHIT_GET_COLLIDER                0x34D8B70
#define OFFSET_COMPONENT_GET_IN_PARENT                0x3474410
#define OFFSET_COMPONENT_GET_TAG                      0x3474760
#define OFFSET_SURFACE_TYPE_FROM_TAG                  0xA2A1B0
#define OFFSET_COMPONENT_GET_IN_CHILDREN              0x3474380
#define OFFSET_TRANSFORM_GET_PARENT                   0x348AA60
#define OFFSET_MATERIAL_GET_COLOR                   0x3450820
#define OFFSET_MATERIAL_SET_COLOR                   0x3450CB0
#define OFFSET_MATERIAL_SET_RENDERQUEUE             0x3451060
#define OFFSET_MATERIAL_SET_FLOAT                   0x344FFD0
#define OFFSET_MATERIAL_HAS_PROPERTY                0x344FCC0
#define OFFSET_MATERIAL_GET_COLOR_ID                0x344F680
#define OFFSET_MATERIAL_SET_COLOR_ID                0x344FE50
#define OFFSET_SHADER_PROPERTY_TO_ID                0x345B6E0
#define OFFSET_RENDERSETTINGS_GET_FOG               0x345A5D0
#define OFFSET_RENDERSETTINGS_GET_FOG_START         0x345A5A0
#define OFFSET_RENDERSETTINGS_GET_FOG_END           0x345A570
#define OFFSET_RENDERSETTINGS_GET_FOG_COLOR         0x345A4C0
#define OFFSET_RENDERSETTINGS_GET_FOG_DENSITY       0x345A540
#define OFFSET_RENDERSETTINGS_GET_SKYBOX            0x345A600
#define OFFSET_RENDERSETTINGS_SET_SKYBOX            0x345A630
#define OFFSET_GLOVES_SET_ARMS                    0x966490
#define OFFSET_ARMSLOD_SET_VISIBLE                 0xBCCFA0
#define OFFSET_WEAPONRY_TAKE_WEAPON                0xBFA0C0
#define OFFSET_GUNCONTROLLER_FIRE                  0xA21040
#define OFFSET_GUNCONTROLLER_COMMAND               0xA1E030
#define OFFSET_HITCASTER_CAST                      0xEEE090  // HitCaster.vxe<ceq>
#define OFFSET_RAGDOLL_ACTIVATE                    0xBF4810  // RagdollController.nol
#define OFFSET_RAGDOLL_MANAGER_RELEASE             0xBF67B0  // RagdollManager.npc
#define OFFSET_RIGIDBODY_GET_IS_KINEMATIC          0x34D92F0
#define OFFSET_RIGIDBODY_SET_IS_KINEMATIC          0x34D97C0
#define OFFSET_RIGIDBODY_SET_VELOCITY              0x34D9A30
#define OFFSET_RIGIDBODY_SET_ANGULAR_VELOCITY      0x34D95F0
#define OFFSET_AIMVIEW_AWAKE                       0xB283C0
#define OFFSET_AIMVIEW_UPDATE_SNIPER_PANELS         0xB29290
#define OFFSET_HUDVIEW_UPDATE                       0xB45320
#define OFFSET_GAMECHATHUD_SEND_MESSAGE               0xA95F70  // GameChatHud.ywa(string)
#define OFFSET_HITMARKERVIEW_SHOW                    0xB47680
#define OFFSET_HITMARKERVIEW_LOCAL_HIT               0xB47B30  // HitMarkerView.bbru(PlayerController, ccv)
#define OFFSET_CHEAT_RUNTIME_SET_THIRDPERSON       0xAEFED0
#define OFFSET_CHEAT_RUNTIME_SET_BHOP              0xAF0760
#define OFFSET_PLAYERCONTROLLER_COMMAND             0xBD6B50
#define OFFSET_PLAYERCONTROLLER_GET_ACTOR           0xBD4730  // PlayerController.mzj() -> dwg
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_A          0xBEB260  // PlayerManager.ndb(PlayerController)
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_B          0xBEB430  // PlayerManager.ndc(PlayerController)
#define OFFSET_PLAYERMANAGER_PLAYER_EVENT_C          0xBEB460  // PlayerManager.ndd(PlayerController)
#define OFFSET_WEAPONCONTROLLER_GET_ID                0x5BD0A0  // WeaponController.vhi() -> dvu
#define OFFSET_PLAYER_HEALTH_GET_CURRENT               0x8E49D0  // bdl.pbq() after network updates
#define OFFSET_PLAYER_HEALTH_APPLY_A                   0x8E4370  // bdl.pbm(...)
#define OFFSET_PLAYER_HEALTH_APPLY_B                   0x8E4500  // bdl.pbn(...)
#define OFFSET_PLAYER_HEALTH_APPLY_C                   0x8E4740  // bdl.pbo(...)
#define OFFSET_PLAYER_HEALTH_APPLY_D                   0x8E48D0  // bdl.pbp(...)
#define OFFSET_PLAYER_HIT_CONFIRMED_A                  0xC2AB10  // PlayerHitController.obl(bai)
#define OFFSET_PLAYER_HIT_CONFIRMED_B                  0xC2ACD0  // PlayerHitController.obm(bai)
#define OFFSET_DAMAGE_RESULT_GET_HEALTH                0xC2C440  // bai.nzh() / dhsx
#define OFFSET_KEYBOARDCONTROL_BUILD_COMMAND        0xB4A790
#define OFFSET_PLAYERCONTROLS_UPDATE                 0xB4CFB0
#define OFFSET_CAMERAMOVEMENTCONTROLLER_UPDATE       0xB5C790
#define OFFSET_CAMERAMOVEMENTCONTROLLER_FIXEDUPDATE 0xB5C700
#define OFFSET_AIMCONTROLLER_APPLY_SNAPSHOT         0xC19440  // AimController.mwp(AimSnapshot)
#define OFFSET_AIMCONTROLLER_GET_SNAPSHOT           0xC194C0
#define OFFSET_AIMCONTROLLER_SET_HEAD_DIRECTIVE      0xC19D80  // AimController.och(Vector3)
#define OFFSET_AIMCONTROLLER_GET_HEAD_DIRECTIVE      0xC19DD0  // AimController.oci() -> Vector3
#define OFFSET_CAMERA_FIRE_ON_PRE_CULL                0x3441780 // Camera.FireOnPreCull(Camera), final render boundary
#define OFFSET_AIMINGDATA_CLONE                      0xC1D170
#define OFFSET_TRANSFORM_GET_EULERANGLES            0x348B3C0
#define OFFSET_TRANSFORM_SET_EULERANGLES            0x348BD30
#define OFFSET_TRANSFORM_GET_LOCALEULERANGLES       0x348B5A0
#define OFFSET_TRANSFORM_SET_LOCALEULERANGLES       0x348BE10
#define OFFSET_COMPONENT_GET_GAMEOBJECT             0x3474720
#define OFFSET_OBJECT_INSTANTIATE                    0x347BAC0
#define OFFSET_OBJECT_DESTROY                        0x347B060
#define OFFSET_TRANSFORM_GET_LOCALSCALE              0x348B800
#define OFFSET_TRANSFORM_SET_LOCALSCALE              0x348C030
#define OFFSET_GAMEOBJECT_SETACTIVE                 0x3478090
#define OFFSET_GAMEOBJECT_GET_ACTIVEINHIERARCHY     0x3478490
#define OFFSET_CANVASGROUP_SET_ALPHA                0x36ABFD0

#define OFFSET_WORLDTOSCREENPOINT                  0x3442780



// Camera matrix getters

#define OFFSET_CAMERA_GET_WORLDTOCAMERAMATRIX      0x3443520

#define OFFSET_CAMERA_GET_PROJECTIONMATRIX         0x34431D0



// WeaponryController offsets

// PlayerController.<bzwy>k__BackingField (WeaponryController) at 0xD0

// dump1 WeaponryController.<cbbc>k__BackingField (current WeaponController) at 0x98

#define OFFSET_WEAPONRYCONTROLLER                  0xD0

#define OFFSET_WEAPONCONTROLLER                    0x98



// GunController fields (short ammo at 0xE4 / 0xE6)

#define OFFSET_RESERVE_AMMO                        0xE4  // short cegx / wyw()
#define OFFSET_CURRENT_AMMO                        0xE6  // short cegy / wyy()




// GunController methods (wyw() / wyy())

#define OFFSET_GUNCONTROLLER_GETCURRENTAMMO        0xA1F050  // vze() -> cgub, current magazine




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

bool pixelSurf = false; // legacy alias; real Pixel Surf state is jbActive

bool jbActive = false;
bool pixelSurfReleaseHop = false;
volatile LONG64 pixelSurfReleasedAt = 0;
volatile LONG64 pixelSurfReleaseGraceUntil = 0;
uintptr_t pixelSurfCharacterController = 0;
volatile LONG pendingPixelSurfChatMessage = 0;

static void SetPixelSurfActive(bool active)
{
    const bool wasActive = jbActive;
    jbActive = active;
    pixelSurf = active;
    if (active) {
        InterlockedExchange64(&pixelSurfReleasedAt, 0);
        if (!wasActive) InterlockedExchange(&pendingPixelSurfChatMessage, 1);
        InterlockedExchange64(&pixelSurfReleaseGraceUntil, 0);
    }
    else if (wasActive) {
        const LONG64 now = static_cast<LONG64>(GetTickCount64());
        InterlockedExchange64(&pixelSurfReleasedAt, now);
        // Optional legacy mode keeps the small release hop the user liked.
        InterlockedExchange64(&pixelSurfReleaseGraceUntil,
            pixelSurfReleaseHop ? now + 100 : 0);
    }
}

bool airJump = false;

bool edgeBugEnabled = false;
float edgeBugPullForce = 20.0f;

bool showVelocity = false;

bool showTrail = false;

bool cameraFovEnabled = false;
float cameraFov = 90.0f;
bool silentAntiAimEnabled = false;
int silentAntiAimMode = 0; // 0 = Static, 1 = Jitter, 2 = Spin
int silentAntiAimPitch = 0; // 0 = Neutral, 1 = Down, 2 = Up
float silentAntiAimYaw = 180.0f;
float silentAntiAimJitterRange = 45.0f;
float silentAntiAimSpinSpeed = 180.0f; // degrees per second
bool silentAntiAimHookReady = false;
volatile LONG silentAntiAimInputBuildCalls = 0;
volatile LONG silentAntiAimCommandCalls = 0;
volatile LONG silentAntiAimAppliedCalls = 0;
bool silentAntiAimLatestInputValid = false;
bool silentAntiAimLatestFiring = false;
uintptr_t silentAntiAimLatestPlayer = 0;
uintptr_t silentAntiAimLatestController = 0;
Vector3 silentAntiAimLatestRealAimAngle;
Vector3 silentAntiAimLatestRealAimEuler;
Vector3 silentAntiAimLatestFakeAngles;
Vector3 silentAntiAimRealCameraAngles;
bool silentAntiAimRealCameraValid = false;
volatile LONG silentAntiAimCameraRestoreCalls = 0;
ULONGLONG silentAntiAimLastSpinTick = 0;
float silentAntiAimSpinYaw = 0.0f;
bool silentAntiAimJitterFlip = false;
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
void(__fastcall* o_CheatRuntime_SetThirdPerson)(uintptr_t, bool, const Il2CppMethod*) = nullptr;
void(__fastcall* o_CheatRuntime_SetBhop)(uintptr_t, uintptr_t, bool) = nullptr;
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

bool penetrableSurfacesEnabled = false;
float penetrableSurfaceFillColor[3] = { 0.05f, 0.80f, 1.0f };
float penetrableSurfaceOutlineColor[3] = { 1.0f, 0.82f, 0.12f };
float penetrableSurfaceAlpha = 0.18f;
float penetrableSurfaceScanDistance = 100.0f;
ULONGLONG penetrableSurfaceNextScan = 0;
size_t penetrableSurfaceScanCursor = 0;
uintptr_t penetrableSurfaceMaterial = 0;
uintptr_t penetrableSurfaceOutlineMaterial = 0;
uint32_t penetrableSurfaceMaterialHandle = 0;
uint32_t penetrableSurfaceOutlineMaterialHandle = 0;
struct PenetrableSurfaceVisual {
    uintptr_t renderer;
    uint32_t rendererHandle;
    std::vector<uintptr_t> originalMaterials;
    uintptr_t outlineObject;
    uintptr_t outlineRenderer;
    int surfaceType;
    ULONGLONG lastSeen;
};
std::vector<PenetrableSurfaceVisual> penetrableSurfaceVisuals;
char penetrableSurfaceStatus[128] = "Disabled";

bool fogEnabled = false;
float fogColor[3] = { 0.55f, 0.62f, 0.72f };
float fogStartDistance = 0.0f;
float fogEndDistance = 120.0f;
float fogDensity = 0.01f;
volatile LONG pendingFogCommand = 0;
volatile LONG fogApplyCalls = 0;
volatile LONG fogRestoreCalls = 0;
ULONGLONG fogLastApplyTick = 0;
bool fogOriginalCaptured = false;
bool fogOriginalEnabled = false;
float fogOriginalStartDistance = 0.0f;
float fogOriginalEndDistance = 0.0f;
float fogOriginalDensity = 0.0f;
Color fogOriginalColor;
char fogStatus[128] = "Setter API not resolved";

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

bool adminBhopEnabled = false;
bool adminBhopCsStrafeMode = false;
volatile LONG adminBhopManualStrafeHeld = 0;
volatile LONG64 adminBhopManualStrafeGraceUntil = 0;
float adminBhopPreviousCameraYaw = 0.0f;
bool adminBhopCameraYawValid = false;
Vector3 adminBhopLastManualMotion;
bool adminBhopLastManualMotionValid = false;
float adminBhopSpeedMultiplier = 1.0f;
float adminBhopMaxSpeed = 6.50f;
uintptr_t adminBhopObservedMovement = 0;
uintptr_t adminBhopObservedJumpParameters = 0;
bool adminBhopOriginalCaptured = false;
bool adminBhopOriginalEnabled = false;
float adminBhopOriginalSpeedMultiplier = 1.0f;
float adminBhopOriginalMaxSpeed = 17.49f;
char adminBhopStatus[96] = "Disabled";

bool infinityAmmo = false;
bool doubleTapEnabled = false;
volatile LONG doubleTapExtraShots = 0;
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
bool aimbotAutoWall = false;
float aimbotAutoWallMinDamage = 1.0f;
float aimbotFov = 360.0f;
float visibleAimbotHoldMs = 140.0f;
bool aimbotAutoFire = false;
uintptr_t aimbotLastHitParameters = 0;
uint32_t aimbotLastHitParametersHandle = 0;
volatile LONG aimbotAutoFired = 0;
volatile LONG aimbotAutoFireRejected = 0;
volatile LONG aimbotAutoFireBusy = 0;
volatile LONG aimbotAutoFireNativeCommands = 0;
uintptr_t aimbotAutoFirePendingGun = 0;
ULONGLONG aimbotAutoFirePendingUntil = 0;
ULONGLONG aimbotAutoFireNextDecisionAt = 0;
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

bool freezeCorpsesEnabled = false;
float freezeCorpsesDuration = 4.0f;
bool freezeCorpsesFadeEnabled = true;
float freezeCorpsesFadeDuration = 1.0f;
int freezeCorpsesChamsMode = 0; // None, Flat, Glass, Lit, Metallic, Rainbow, Pulse, Outline
float freezeCorpsesChamsColor[3] = { 1.0f, 0.20f, 0.35f };
float freezeCorpsesChamsAlpha = 0.55f;
float freezeCorpsesMetallic = 0.9f;
float freezeCorpsesSmoothness = 0.8f;
float freezeCorpsesAnimationSpeed = 1.0f;
struct FrozenCorpseEntry {
    uintptr_t ragdoll;
    uintptr_t manager;
    ULONGLONG releaseAt;
    bool releaseRequested;
    uintptr_t renderer;
    uintptr_t originalMaterial;
    uintptr_t fadeMaterial;
    uint32_t fadeMaterialHandle;
    uintptr_t outlineObject;
    uintptr_t outlineRenderer;
    int materialMode;
    ULONGLONG createdAt;
    std::vector<std::pair<uintptr_t, bool>> rigidbodies;
};
std::vector<FrozenCorpseEntry> frozenCorpses;
SRWLOCK frozenCorpseLock = SRWLOCK_INIT;

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
bool hitLogEnabled = false;
float hitLogDuration = 4.0f;
struct HitLogEntry {
    char enemyName[96];
    int damage;
    char hitbox[16];
    ULONGLONG createdAt;
};
std::vector<HitLogEntry> hitLogEntries;
SRWLOCK hitLogLock = SRWLOCK_INIT;
uintptr_t hitLogLastResult = 0;
ULONGLONG hitLogLastResultAt = 0;
bool bulletTracerEnabled = false;
bool bulletImpactsEnabled = false;
float bulletImpactsDuration = 4.0f;
float bulletImpactsSize = 0.08f;
float bulletImpactsClientColor[3] = { 1.0f, 0.215f, 0.215f };
float bulletImpactsConfirmedColor[3] = { 0.176f, 0.49f, 1.0f };
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
int weaponChamsMode = 0; // Flat, Glass, Lit, Metallic, Transparent Lit, Rainbow, Pulse, Outline
float weaponChamsColor[3] = { 0.05f, 0.75f, 1.0f };
float weaponChamsGlassAlpha = 0.35f;
float weaponChamsMetallic = 0.9f;
float weaponChamsSmoothness = 0.8f;
float weaponChamsAnimationSpeed = 1.0f;
ULONGLONG weaponChamsLastAnimationTick = 0;

struct WeaponChamsRenderer {
    uintptr_t renderer;
    uintptr_t originalMaterial;
    uintptr_t outlineObject;
    uintptr_t outlineRenderer;
};
std::vector<WeaponChamsRenderer> weaponChamsRenderers;
uintptr_t weaponChamsController = 0;
uintptr_t activeLocalWeaponController = 0;
uintptr_t chamsObservedLocalPlayer = 0;
uintptr_t weaponChamsMaterials[8] = {};
uint32_t weaponChamsMaterialHandles[8] = {};
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
uintptr_t armChamsMaterials[8] = {};
uint32_t armChamsMaterialHandles[8] = {};
struct ArmChamsRenderer {
    uintptr_t renderer;
    uintptr_t originalMaterial;
    uintptr_t outlineObject;
    uintptr_t outlineRenderer;
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
uintptr_t gloveChamsMaterials[8] = {};
uint32_t gloveChamsMaterialHandles[8] = {};
struct GloveChamsRenderer { uintptr_t renderer; uintptr_t originalMaterial; uintptr_t outlineObject; uintptr_t outlineRenderer; };
std::vector<GloveChamsRenderer> gloveChamsRenderers;
uintptr_t gloveChamsArmsLodGroup = 0;
char gloveChamsStatus[128] = "Disabled";
volatile LONG pendingGloveChamsRefresh = 0;

bool localPlayerChamsEnabled = false;
int localPlayerChamsMode = 0;
float localPlayerChamsColor[3] = { 0.15f, 0.75f, 1.0f };
float localPlayerChamsAlpha = 0.35f;
float localPlayerChamsMetallic = 0.9f;
float localPlayerChamsSmoothness = 0.8f;
float localPlayerChamsAnimationSpeed = 1.0f;
ULONGLONG localPlayerChamsLastAnimationTick = 0;
uintptr_t localPlayerChamsMaterials[8] = {};
uint32_t localPlayerChamsMaterialHandles[8] = {};
struct LocalPlayerChamsRenderer { uintptr_t renderer; uintptr_t originalMaterial; uintptr_t outlineObject; uintptr_t outlineRenderer; };
std::vector<LocalPlayerChamsRenderer> localPlayerChamsRenderers;
uintptr_t localPlayerChamsLodGroup = 0;
char localPlayerChamsStatus[128] = "Disabled";
volatile LONG pendingLocalPlayerChamsRefresh = 0;

bool enemyChamsEnabled = false;
bool enemyChamsThroughWalls = false;
int enemyChamsMode = 0;
float enemyChamsColor[3] = { 0.45f, 0.20f, 1.0f };
float enemyChamsAlpha = 0.35f;
float enemyChamsMetallic = 0.9f;
float enemyChamsSmoothness = 0.8f;
float enemyChamsAnimationSpeed = 1.0f;
ULONGLONG enemyChamsLastAnimationTick = 0;
uintptr_t enemyChamsMaterials[9] = {};
uint32_t enemyChamsMaterialHandles[9] = {};
struct EnemyChamsRenderer { uintptr_t renderer; uintptr_t originalMaterial; bool originalEnabled; uintptr_t outlineObject; uintptr_t outlineRenderer; };
std::vector<EnemyChamsRenderer> enemyChamsRenderers;
std::vector<uintptr_t> enemyChamsInitializedOcclusionControllers;
bool enemyChamsThroughWallsObserved = false;
uintptr_t currentLocalCharacterLodGroupCache = 0;
char enemyChamsStatus[128] = "Disabled";
volatile LONG pendingEnemyChamsRefresh = 0;
ULONGLONG chamsLastMaintenanceTick = 0;

struct BulletTracerEntry {
    Vector3 start;
    Vector3 end;
    ULONGLONG createdAt;
};
std::vector<BulletTracerEntry> bulletTracers;
SRWLOCK bulletTracerLock = SRWLOCK_INIT;
struct BulletImpactEntry {
    Vector3 position;
    ULONGLONG createdAt;
    bool serverConfirmed;
};
std::vector<BulletImpactEntry> bulletImpacts;
SRWLOCK bulletImpactLock = SRWLOCK_INIT;



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
static void* GetLocalPC();
static unsigned char GetPCTeam(void* pc);
static void GetPCName(void* pc, char* output, int outputSize);
static int GetPCHealth(void* pc);

static void CacheUpdatedHealth(uintptr_t state) {
    const int hp = ReadHealthStateNow(state);
    if (hp < 0) return;
    uintptr_t player = 0;
    __try { player = *reinterpret_cast<uintptr_t*>(state + 0x40); } // bdl.ccgy
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

static void* GetLocalPC();
static void GetPCName(void* pc, char* output, int outputSize);
static void CollectPlayers(void** out, int maxN, int& outN);

static const char* HitboxNameFromDamageCounts(int headHits, int bodyHits, int feetHits) {
    if (headHits > 0) return "Head";
    if (bodyHits > 0) return "Body";
    if (feetHits > 0) return "Legs";
    return "Unknown";
}

static uintptr_t SafeGetPlayerHitActor(void* player) {
    if (!player) return 0;
    __try {
        const uintptr_t hitController = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(player) + 0xF0);
        return hitController ? *reinterpret_cast<uintptr_t*>(hitController + 0xD0) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
static uintptr_t SafeGetPlayerActor(void* player) {
    if (!player || !base) return 0;
    __try {
        using GetActorFn = uintptr_t(__fastcall*)(void*, const Il2CppMethod*);
        static GetActorFn getActor = nullptr;
        if (!getActor)
            getActor = reinterpret_cast<GetActorFn>(base + OFFSET_PLAYERCONTROLLER_GET_ACTOR);
        return getActor(player, nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static bool ActorBelongsToPlayer(uintptr_t actor, void* player) {
    if (!actor || !player) return false;
    return actor == SafeGetPlayerHitActor(player) || actor == SafeGetPlayerActor(player);
}

static void* FindPlayerByActor(uintptr_t actor) {
    if (!actor) return nullptr;
    void* players[64] = {};
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    for (int i = 0; i < playerCount; ++i) {
        if (players[i] && ActorBelongsToPlayer(actor, players[i]))
            return players[i];
    }
    return nullptr;
}

static void CacheConfirmedDamageResult(uintptr_t hitController, uintptr_t result) {
    // dump1 changed the damage-result hierarchy/layout (boo/boq -> bai/bak).
    // Keep hook counters only; do not dereference the old result fields.
    if (!hitController || !result) return;
    InterlockedIncrement(&boxEspConfirmedHits);
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
        const uintptr_t weapon = weaponry ? *reinterpret_cast<uintptr_t*>(weaponry + OFFSET_WEAPONCONTROLLER) : 0;
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
char configName[64] = "default";
char configStatus[128] = "Ready";
std::vector<std::string> configFiles;

float menuColor[3] = { 0.066f, 0.059f, 0.141f }; // Fatality window_bg

float accentColor[3] = { 0.761f, 0.09f, 0.314f }; // Fatality selection

bool useCustomBackground = false;

ID3D11ShaderResourceView* backgroundTexture = nullptr;

bool backgroundLoaded = false;

char backgroundPath[256] = "maxresdefault.jpg"; // Default path

static std::wstring GetConfigDirectory()
{
    PWSTR documents = nullptr;
    std::wstring directory;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE,
        nullptr, &documents)) && documents) {
        directory = documents;
        directory += L"\\ze0nware";
    }
    if (documents) CoTaskMemFree(documents);
    if (directory.empty()) directory = L".\\ze0nware";
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory;
}

static std::string SanitizeConfigName(const char* input)
{
    std::string result;
    for (const char* c = input; c && *c && result.size() < 48; ++c)
        if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') ||
            (*c >= '0' && *c <= '9') || *c == '-' || *c == '_') result += *c;
    return result.empty() ? "default" : result;
}

static std::wstring AsciiToWide(const std::string& text)
{ return std::wstring(text.begin(), text.end()); }

static std::wstring GetConfigPath(const char* name)
{
    return GetConfigDirectory() + L"\\" + AsciiToWide(SanitizeConfigName(name)) + L".ini";
}

static void RefreshConfigFiles()
{
    configFiles.clear();
    WIN32_FIND_DATAW data = {};
    HANDLE find = FindFirstFileW((GetConfigDirectory() + L"\\*.ini").c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        std::wstring wideName = data.cFileName;
        if (wideName.size() > 4) wideName.resize(wideName.size() - 4);
        configFiles.emplace_back(wideName.begin(), wideName.end());
    } while (FindNextFileW(find, &data));
    FindClose(find);
    std::sort(configFiles.begin(), configFiles.end());
}

static std::wstring WideKey(const char* key)
{ std::string text(key); return std::wstring(text.begin(), text.end()); }
static void WriteConfigValue(const std::wstring& path, const char* key, bool value)
{ const std::wstring k = WideKey(key); WritePrivateProfileStringW(L"ze0nware", k.c_str(), value ? L"1" : L"0", path.c_str()); }
static void WriteConfigValue(const std::wstring& path, const char* key, int value)
{ wchar_t text[32]; swprintf_s(text, L"%d", value); const std::wstring k = WideKey(key); WritePrivateProfileStringW(L"ze0nware", k.c_str(), text, path.c_str()); }
static void WriteConfigValue(const std::wstring& path, const char* key, float value)
{ wchar_t text[32]; swprintf_s(text, L"%.6f", value); const std::wstring k = WideKey(key); WritePrivateProfileStringW(L"ze0nware", k.c_str(), text, path.c_str()); }
static bool ReadConfigBool(const std::wstring& path, const char* key, bool fallback)
{ const std::wstring k = WideKey(key); return GetPrivateProfileIntW(L"ze0nware", k.c_str(), fallback ? 1 : 0, path.c_str()) != 0; }
static int ReadConfigInt(const std::wstring& path, const char* key, int fallback)
{ const std::wstring k = WideKey(key); return GetPrivateProfileIntW(L"ze0nware", k.c_str(), fallback, path.c_str()); }
static float ReadConfigFloat(const std::wstring& path, const char* key, float fallback)
{
    wchar_t text[64] = {}, fallbackText[32] = {};
    swprintf_s(fallbackText, L"%.6f", fallback);
    const std::wstring k = WideKey(key);
    GetPrivateProfileStringW(L"ze0nware", k.c_str(), fallbackText, text,
        static_cast<DWORD>(sizeof(text) / sizeof(text[0])), path.c_str());
    const float value = static_cast<float>(_wtof(text));
    return isfinite(value) ? value : fallback;
}

static void WriteConfigColor(const std::wstring& path, const char* key, const float color[3])
{
    char component[96];
    for (int i = 0; i < 3; ++i) {
        sprintf_s(component, "%s_%d", key, i);
        WriteConfigValue(path, component, color[i]);
    }
}
static void ReadConfigColor(const std::wstring& path, const char* key, float color[3])
{
    char component[96];
    for (int i = 0; i < 3; ++i) {
        sprintf_s(component, "%s_%d", key, i);
        color[i] = ReadConfigFloat(path, component, color[i]);
        if (color[i] < 0.0f) color[i] = 0.0f;
        if (color[i] > 1.0f) color[i] = 1.0f;
    }
}

static bool ApplyAdminBhopState();

static bool SaveConfig(const char* requestedName)
{
    const std::string name = SanitizeConfigName(requestedName);
    const std::wstring path = GetConfigPath(name.c_str());
#define SAVE_BOOL(v) WriteConfigValue(path, #v, v)
#define SAVE_INT(v) WriteConfigValue(path, #v, v)
#define SAVE_FLOAT(v) WriteConfigValue(path, #v, v)
    SAVE_BOOL(jbActive); SAVE_BOOL(pixelSurfReleaseHop); SAVE_FLOAT(surfSpeed);
    SAVE_BOOL(adminBhopEnabled); SAVE_BOOL(adminBhopCsStrafeMode); SAVE_FLOAT(adminBhopMaxSpeed);
    SAVE_BOOL(airJump); SAVE_BOOL(edgeBugEnabled); SAVE_FLOAT(edgeBugPullForce);
    SAVE_BOOL(velocityLimiterEnabled); SAVE_FLOAT(velocityLimit);
    SAVE_BOOL(aimbotEnabled); SAVE_BOOL(visibleAimbotEnabled); SAVE_BOOL(aimbotVisibleCheck); SAVE_BOOL(aimbotAutoWall); SAVE_FLOAT(aimbotAutoWallMinDamage); SAVE_BOOL(aimbotAutoFire);
    SAVE_BOOL(silentAntiAimEnabled); SAVE_INT(silentAntiAimMode); SAVE_INT(silentAntiAimPitch); SAVE_FLOAT(silentAntiAimYaw); SAVE_FLOAT(silentAntiAimJitterRange); SAVE_FLOAT(silentAntiAimSpinSpeed); SAVE_BOOL(freezeCorpsesEnabled); SAVE_FLOAT(freezeCorpsesDuration); SAVE_BOOL(freezeCorpsesFadeEnabled); SAVE_FLOAT(freezeCorpsesFadeDuration); SAVE_INT(freezeCorpsesChamsMode); SAVE_FLOAT(freezeCorpsesChamsAlpha); SAVE_FLOAT(freezeCorpsesMetallic); SAVE_FLOAT(freezeCorpsesSmoothness); SAVE_FLOAT(freezeCorpsesAnimationSpeed); SAVE_BOOL(boxEsp); SAVE_INT(espCount); SAVE_FLOAT(espMaxDistance);
    SAVE_BOOL(espShowName); SAVE_BOOL(espShowHealth); SAVE_BOOL(espShowWeapon); SAVE_BOOL(espGradient);
    SAVE_BOOL(worldColorEnabled); SAVE_INT(worldColorMode); SAVE_FLOAT(worldColorStrength); SAVE_FLOAT(worldColorAlpha);
    SAVE_BOOL(penetrableSurfacesEnabled); SAVE_FLOAT(penetrableSurfaceAlpha); SAVE_FLOAT(penetrableSurfaceScanDistance);
    SAVE_BOOL(fogEnabled); SAVE_FLOAT(fogStartDistance); SAVE_FLOAT(fogEndDistance); SAVE_FLOAT(fogDensity);
    SAVE_BOOL(thirdPersonEnabled); SAVE_FLOAT(thirdPersonHorizontalOffset); SAVE_FLOAT(thirdPersonHeightAdjustment); SAVE_FLOAT(thirdPersonDistanceAdjustment);
    SAVE_BOOL(infinityAmmo); SAVE_BOOL(doubleTapEnabled); SAVE_BOOL(noSpreadEnabled); SAVE_BOOL(removeScopeBorders);
    SAVE_BOOL(weaponChamsEnabled); SAVE_INT(weaponChamsMode); SAVE_BOOL(armChamsEnabled); SAVE_INT(armChamsMode);
    SAVE_BOOL(gloveChamsEnabled); SAVE_INT(gloveChamsMode); SAVE_BOOL(localPlayerChamsEnabled); SAVE_INT(localPlayerChamsMode);
    SAVE_BOOL(enemyChamsEnabled); SAVE_BOOL(enemyChamsThroughWalls); SAVE_INT(enemyChamsMode);
    SAVE_BOOL(hitMarkerEnabled); SAVE_FLOAT(hitMarkerDuration); SAVE_FLOAT(hitMarkerSize); SAVE_FLOAT(hitMarkerGap); SAVE_FLOAT(hitMarkerThickness);
    SAVE_BOOL(hitLogEnabled); SAVE_FLOAT(hitLogDuration);
    SAVE_BOOL(bulletTracerEnabled); SAVE_FLOAT(bulletTracerDuration); SAVE_FLOAT(bulletTracerThickness);
    SAVE_BOOL(bulletImpactsEnabled); SAVE_FLOAT(bulletImpactsDuration); SAVE_FLOAT(bulletImpactsSize);
    SAVE_BOOL(showVelocity); SAVE_BOOL(showTrail); SAVE_INT(maxTrailPoints); SAVE_FLOAT(trailMinDistance);
    SAVE_BOOL(cameraFovEnabled); SAVE_FLOAT(cameraFov); SAVE_FLOAT(aimbotFov); SAVE_FLOAT(visibleAimbotHoldMs);
    SAVE_BOOL(espHealthGradient);
    SAVE_FLOAT(worldColorMetallic); SAVE_FLOAT(worldColorSmoothness); SAVE_FLOAT(worldColorAnimationSpeed);
    SAVE_BOOL(customSkyboxEnabled);
    SAVE_FLOAT(weaponChamsGlassAlpha); SAVE_FLOAT(weaponChamsMetallic); SAVE_FLOAT(weaponChamsSmoothness); SAVE_FLOAT(weaponChamsAnimationSpeed);
    SAVE_FLOAT(armChamsAlpha); SAVE_FLOAT(armChamsMetallic); SAVE_FLOAT(armChamsSmoothness); SAVE_FLOAT(armChamsAnimationSpeed);
    SAVE_FLOAT(gloveChamsAlpha); SAVE_FLOAT(gloveChamsMetallic); SAVE_FLOAT(gloveChamsSmoothness); SAVE_FLOAT(gloveChamsAnimationSpeed);
    SAVE_FLOAT(localPlayerChamsAlpha); SAVE_FLOAT(localPlayerChamsMetallic); SAVE_FLOAT(localPlayerChamsSmoothness); SAVE_FLOAT(localPlayerChamsAnimationSpeed);
    SAVE_FLOAT(enemyChamsAlpha); SAVE_FLOAT(enemyChamsMetallic); SAVE_FLOAT(enemyChamsSmoothness); SAVE_FLOAT(enemyChamsAnimationSpeed);
    SAVE_INT(psBind.key); SAVE_BOOL(psBind.toggleMode); SAVE_INT(jbBind.key); SAVE_BOOL(jbBind.toggleMode);
    SAVE_INT(airJumpBind.key); SAVE_BOOL(airJumpBind.toggleMode); SAVE_INT(edgeBugBind.key); SAVE_BOOL(edgeBugBind.toggleMode);
    SAVE_INT(velocityBind.key); SAVE_BOOL(velocityBind.toggleMode); SAVE_INT(trailBind.key); SAVE_BOOL(trailBind.toggleMode);
    SAVE_INT(ammoBind.key); SAVE_BOOL(ammoBind.toggleMode); SAVE_INT(espBind.key); SAVE_BOOL(espBind.toggleMode);
    WriteConfigColor(path, "menuColor", menuColor); WriteConfigColor(path, "accentColor", accentColor);
    WriteConfigColor(path, "worldColor", worldColor); WriteConfigColor(path, "fogColor", fogColor);
    WriteConfigColor(path, "penetrableSurfaceFillColor", penetrableSurfaceFillColor);
    WriteConfigColor(path, "penetrableSurfaceOutlineColor", penetrableSurfaceOutlineColor);
    WriteConfigColor(path, "customSkyboxColor", customSkyboxColor);
    WriteConfigColor(path, "hitMarkerColor", hitMarkerColor);
    WriteConfigColor(path, "freezeCorpsesChamsColor", freezeCorpsesChamsColor);
    WriteConfigColor(path, "bulletTracerStartColor", bulletTracerStartColor); WriteConfigColor(path, "bulletTracerEndColor", bulletTracerEndColor);
    WriteConfigColor(path, "bulletImpactsClientColor", bulletImpactsClientColor); WriteConfigColor(path, "bulletImpactsConfirmedColor", bulletImpactsConfirmedColor);
    WriteConfigColor(path, "weaponChamsColor", weaponChamsColor); WriteConfigColor(path, "armChamsColor", armChamsColor);
    WriteConfigColor(path, "gloveChamsColor", gloveChamsColor); WriteConfigColor(path, "localPlayerChamsColor", localPlayerChamsColor);
    WriteConfigColor(path, "enemyChamsColor", enemyChamsColor);
    WriteConfigColor(path, "espTopColor", espTopColor); WriteConfigColor(path, "espBottomColor", espBottomColor);
    WriteConfigColor(path, "espNameColor", espNameColor); WriteConfigColor(path, "espHealthColor", espHealthColor);
    WriteConfigColor(path, "espHealthBottomColor", espHealthBottomColor);
#undef SAVE_BOOL
#undef SAVE_INT
#undef SAVE_FLOAT
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, path.c_str());
    strcpy_s(configName, name.c_str());
    sprintf_s(configStatus, "Saved %s", name.c_str());
    RefreshConfigFiles();
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static bool LoadConfig(const char* requestedName)
{
    const std::string name = SanitizeConfigName(requestedName);
    const std::wstring path = GetConfigPath(name.c_str());
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        sprintf_s(configStatus, "Config %s not found", name.c_str());
        return false;
    }
#define LOAD_BOOL(v) v = ReadConfigBool(path, #v, v)
#define LOAD_INT(v) v = ReadConfigInt(path, #v, v)
#define LOAD_FLOAT(v) v = ReadConfigFloat(path, #v, v)
    LOAD_BOOL(jbActive); pixelSurf = jbActive; LOAD_BOOL(pixelSurfReleaseHop); LOAD_FLOAT(surfSpeed);
    LOAD_BOOL(adminBhopEnabled); LOAD_BOOL(adminBhopCsStrafeMode); LOAD_FLOAT(adminBhopMaxSpeed);
    if (adminBhopMaxSpeed < 1.0f) adminBhopMaxSpeed = 1.0f; if (adminBhopMaxSpeed > 30.0f) adminBhopMaxSpeed = 30.0f;
    LOAD_BOOL(airJump); LOAD_BOOL(edgeBugEnabled); LOAD_FLOAT(edgeBugPullForce);
    LOAD_BOOL(velocityLimiterEnabled); LOAD_FLOAT(velocityLimit);
    LOAD_BOOL(aimbotEnabled); LOAD_BOOL(visibleAimbotEnabled); LOAD_BOOL(aimbotVisibleCheck); LOAD_BOOL(aimbotAutoWall); LOAD_FLOAT(aimbotAutoWallMinDamage); LOAD_BOOL(aimbotAutoFire);
    LOAD_BOOL(silentAntiAimEnabled); LOAD_INT(silentAntiAimMode); LOAD_INT(silentAntiAimPitch); LOAD_FLOAT(silentAntiAimYaw); LOAD_FLOAT(silentAntiAimJitterRange); LOAD_FLOAT(silentAntiAimSpinSpeed); if (silentAntiAimMode < 0 || silentAntiAimMode > 2) silentAntiAimMode = 0; if (silentAntiAimPitch < 0 || silentAntiAimPitch > 2) silentAntiAimPitch = 0; if (silentAntiAimYaw < -180.0f) silentAntiAimYaw = -180.0f; if (silentAntiAimYaw > 180.0f) silentAntiAimYaw = 180.0f; if (silentAntiAimJitterRange < 0.0f) silentAntiAimJitterRange = 0.0f; if (silentAntiAimJitterRange > 180.0f) silentAntiAimJitterRange = 180.0f; if (silentAntiAimSpinSpeed < 1.0f) silentAntiAimSpinSpeed = 1.0f; if (silentAntiAimSpinSpeed > 1080.0f) silentAntiAimSpinSpeed = 1080.0f; LOAD_BOOL(freezeCorpsesEnabled); LOAD_FLOAT(freezeCorpsesDuration); LOAD_BOOL(freezeCorpsesFadeEnabled); LOAD_FLOAT(freezeCorpsesFadeDuration); LOAD_INT(freezeCorpsesChamsMode); LOAD_FLOAT(freezeCorpsesChamsAlpha); LOAD_FLOAT(freezeCorpsesMetallic); LOAD_FLOAT(freezeCorpsesSmoothness); LOAD_FLOAT(freezeCorpsesAnimationSpeed); LOAD_BOOL(boxEsp); LOAD_INT(espCount); LOAD_FLOAT(espMaxDistance);
    LOAD_BOOL(espShowName); LOAD_BOOL(espShowHealth); LOAD_BOOL(espShowWeapon); LOAD_BOOL(espGradient);
    LOAD_BOOL(worldColorEnabled); LOAD_INT(worldColorMode); LOAD_FLOAT(worldColorStrength); LOAD_FLOAT(worldColorAlpha);
    LOAD_BOOL(penetrableSurfacesEnabled); LOAD_FLOAT(penetrableSurfaceAlpha); LOAD_FLOAT(penetrableSurfaceScanDistance);
    if (penetrableSurfaceAlpha < 0.05f) penetrableSurfaceAlpha = 0.05f; if (penetrableSurfaceAlpha > 0.80f) penetrableSurfaceAlpha = 0.80f;
    if (penetrableSurfaceScanDistance < 10.0f) penetrableSurfaceScanDistance = 10.0f; if (penetrableSurfaceScanDistance > 250.0f) penetrableSurfaceScanDistance = 250.0f;
    LOAD_BOOL(fogEnabled); LOAD_FLOAT(fogStartDistance); LOAD_FLOAT(fogEndDistance); LOAD_FLOAT(fogDensity);
    LOAD_BOOL(thirdPersonEnabled); LOAD_FLOAT(thirdPersonHorizontalOffset); LOAD_FLOAT(thirdPersonHeightAdjustment); LOAD_FLOAT(thirdPersonDistanceAdjustment);
    LOAD_BOOL(infinityAmmo); LOAD_BOOL(doubleTapEnabled); LOAD_BOOL(noSpreadEnabled); LOAD_BOOL(removeScopeBorders);
    LOAD_BOOL(weaponChamsEnabled); LOAD_INT(weaponChamsMode); LOAD_BOOL(armChamsEnabled); LOAD_INT(armChamsMode);
    LOAD_BOOL(gloveChamsEnabled); LOAD_INT(gloveChamsMode); LOAD_BOOL(localPlayerChamsEnabled); LOAD_INT(localPlayerChamsMode);
    LOAD_BOOL(enemyChamsEnabled); LOAD_BOOL(enemyChamsThroughWalls); LOAD_INT(enemyChamsMode);
    LOAD_BOOL(hitMarkerEnabled); LOAD_FLOAT(hitMarkerDuration); LOAD_FLOAT(hitMarkerSize); LOAD_FLOAT(hitMarkerGap); LOAD_FLOAT(hitMarkerThickness);
    LOAD_BOOL(hitLogEnabled); LOAD_FLOAT(hitLogDuration);
    LOAD_BOOL(bulletTracerEnabled); LOAD_FLOAT(bulletTracerDuration); LOAD_FLOAT(bulletTracerThickness);
    LOAD_BOOL(bulletImpactsEnabled); LOAD_FLOAT(bulletImpactsDuration); LOAD_FLOAT(bulletImpactsSize);
    LOAD_BOOL(showVelocity); LOAD_BOOL(showTrail); LOAD_INT(maxTrailPoints); LOAD_FLOAT(trailMinDistance);
    LOAD_BOOL(cameraFovEnabled); LOAD_FLOAT(cameraFov); LOAD_FLOAT(aimbotFov); LOAD_FLOAT(visibleAimbotHoldMs);
    LOAD_BOOL(espHealthGradient);
    LOAD_FLOAT(worldColorMetallic); LOAD_FLOAT(worldColorSmoothness); LOAD_FLOAT(worldColorAnimationSpeed);
    LOAD_BOOL(customSkyboxEnabled);
    LOAD_FLOAT(weaponChamsGlassAlpha); LOAD_FLOAT(weaponChamsMetallic); LOAD_FLOAT(weaponChamsSmoothness); LOAD_FLOAT(weaponChamsAnimationSpeed);
    LOAD_FLOAT(armChamsAlpha); LOAD_FLOAT(armChamsMetallic); LOAD_FLOAT(armChamsSmoothness); LOAD_FLOAT(armChamsAnimationSpeed);
    LOAD_FLOAT(gloveChamsAlpha); LOAD_FLOAT(gloveChamsMetallic); LOAD_FLOAT(gloveChamsSmoothness); LOAD_FLOAT(gloveChamsAnimationSpeed);
    LOAD_FLOAT(localPlayerChamsAlpha); LOAD_FLOAT(localPlayerChamsMetallic); LOAD_FLOAT(localPlayerChamsSmoothness); LOAD_FLOAT(localPlayerChamsAnimationSpeed);
    LOAD_FLOAT(enemyChamsAlpha); LOAD_FLOAT(enemyChamsMetallic); LOAD_FLOAT(enemyChamsSmoothness); LOAD_FLOAT(enemyChamsAnimationSpeed);
    LOAD_INT(psBind.key); LOAD_BOOL(psBind.toggleMode); LOAD_INT(jbBind.key); LOAD_BOOL(jbBind.toggleMode);
    LOAD_INT(airJumpBind.key); LOAD_BOOL(airJumpBind.toggleMode); LOAD_INT(edgeBugBind.key); LOAD_BOOL(edgeBugBind.toggleMode);
    LOAD_INT(velocityBind.key); LOAD_BOOL(velocityBind.toggleMode); LOAD_INT(trailBind.key); LOAD_BOOL(trailBind.toggleMode);
    LOAD_INT(ammoBind.key); LOAD_BOOL(ammoBind.toggleMode); LOAD_INT(espBind.key); LOAD_BOOL(espBind.toggleMode);
    ReadConfigColor(path, "menuColor", menuColor); ReadConfigColor(path, "accentColor", accentColor);
    ReadConfigColor(path, "worldColor", worldColor); ReadConfigColor(path, "fogColor", fogColor);
    ReadConfigColor(path, "penetrableSurfaceFillColor", penetrableSurfaceFillColor);
    ReadConfigColor(path, "penetrableSurfaceOutlineColor", penetrableSurfaceOutlineColor);
    ReadConfigColor(path, "customSkyboxColor", customSkyboxColor);
    ReadConfigColor(path, "hitMarkerColor", hitMarkerColor);
    ReadConfigColor(path, "freezeCorpsesChamsColor", freezeCorpsesChamsColor);
    ReadConfigColor(path, "bulletTracerStartColor", bulletTracerStartColor); ReadConfigColor(path, "bulletTracerEndColor", bulletTracerEndColor);
    ReadConfigColor(path, "bulletImpactsClientColor", bulletImpactsClientColor); ReadConfigColor(path, "bulletImpactsConfirmedColor", bulletImpactsConfirmedColor);
    ReadConfigColor(path, "weaponChamsColor", weaponChamsColor); ReadConfigColor(path, "armChamsColor", armChamsColor);
    ReadConfigColor(path, "gloveChamsColor", gloveChamsColor); ReadConfigColor(path, "localPlayerChamsColor", localPlayerChamsColor);
    ReadConfigColor(path, "enemyChamsColor", enemyChamsColor);
    ReadConfigColor(path, "espTopColor", espTopColor); ReadConfigColor(path, "espBottomColor", espBottomColor);
    ReadConfigColor(path, "espNameColor", espNameColor); ReadConfigColor(path, "espHealthColor", espHealthColor);
    ReadConfigColor(path, "espHealthBottomColor", espHealthBottomColor);
#undef LOAD_BOOL
#undef LOAD_INT
#undef LOAD_FLOAT
    adminBhopLastManualMotionValid = false;
    adminBhopCameraYawValid = false;
    InterlockedExchange(&adminBhopManualStrafeHeld, 0);
    InterlockedExchange64(&adminBhopManualStrafeGraceUntil, 0);
    InterlockedExchange(&pendingWorldColorCommand, worldColorEnabled ? 1 : 2);
    InterlockedExchange(&pendingFogCommand, fogEnabled ? 1 : 2);
    InterlockedExchange(&pendingWeaponChamsRefresh, 1); InterlockedExchange(&pendingArmChamsRefresh, 1);
    InterlockedExchange(&pendingGloveChamsRefresh, 1); InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
    InterlockedExchange(&pendingEnemyChamsRefresh, 1); InterlockedExchange(&pendingThirdPersonCommand, 1);
    ApplyAdminBhopState();
    strcpy_s(configName, name.c_str());
    sprintf_s(configStatus, "Loaded %s", name.c_str());
    return true;
}

static bool DeleteConfig(const char* requestedName)
{
    const std::string name = SanitizeConfigName(requestedName);
    const bool removed = DeleteFileW(GetConfigPath(name.c_str()).c_str()) != 0;
    sprintf_s(configStatus, removed ? "Deleted %s" : "Could not delete %s", name.c_str());
    RefreshConfigFiles();
    return removed;
}

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
Vector3(__fastcall* o_Transform_get_localEulerAngles)(uintptr_t) = nullptr;
void(__fastcall* o_Transform_set_localEulerAngles)(uintptr_t, Vector3) = nullptr;

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
bool(__fastcall* o_Renderer_get_enabled)(uintptr_t) = nullptr;
void(__fastcall* o_Renderer_set_enabled)(uintptr_t, bool) = nullptr;
void(__fastcall* o_SkinnedMeshRenderer_set_updateWhenOffscreen)(uintptr_t, bool) = nullptr;
void(__fastcall* o_ObjectOccludee_SetVisibleState)(uintptr_t, bool, const Il2CppMethod*) = nullptr;

void __fastcall hk_ObjectOccludee_SetVisibleState(uintptr_t instance, bool visible, const Il2CppMethod* method)
{
    bool keepEnemyVisible = false;
    if (keyValidated && enemyChamsEnabled && enemyChamsThroughWalls && instance && !visible) {
        __try {
            // PlayerOcclusionController inherits ObjectOccludee and owns its PlayerController at +0x48.
            void* player = *reinterpret_cast<void**>(instance + 0x48);
            void* localPlayer = GetLocalPC();
            const unsigned char localTeam = GetPCTeam(localPlayer);
            const unsigned char team = GetPCTeam(player);
            keepEnemyVisible = player && player != localPlayer && localTeam != 0 && localTeam != 3 &&
                team != 0 && team != 3 && team != localTeam;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { keepEnemyVisible = false; }
    }
    if (!keepEnemyVisible) {
        o_ObjectOccludee_SetVisibleState(instance, visible, method);
        return;
    }

    // Do not merely replace false with true: when the state was already true that became
    // a no-op, so ObjectOccludee never notified its bpj listeners and remote Mecanim stayed
    // frozen. Complete the native hidden transition and immediately restore visible state
    // in the same call. All game listeners receive the proper disable/enable callbacks,
    // while no render frame can occur between the two synchronous transitions.
    o_ObjectOccludee_SetVisibleState(instance, false, method);
    o_ObjectOccludee_SetVisibleState(instance, true, method);
}

Il2CppArray*(__fastcall* o_Object_FindObjectsOfType)(Il2CppObject*, bool, const Il2CppMethod*) = nullptr;
Color(__fastcall* o_Material_get_color)(uintptr_t) = nullptr;
void(__fastcall* o_Material_set_color)(uintptr_t, Color) = nullptr;
void(__fastcall* o_Material_set_renderQueue)(uintptr_t, int) = nullptr;
void(__fastcall* o_Material_SetFloat)(uintptr_t, Il2CppString*, float) = nullptr;
bool(__fastcall* o_Material_HasProperty)(uintptr_t, int) = nullptr;
Color(__fastcall* o_Material_GetColorId)(uintptr_t, int) = nullptr;
void(__fastcall* o_Material_SetColorId)(uintptr_t, int, Color) = nullptr;
int(__fastcall* o_Shader_PropertyToID)(Il2CppString*, const Il2CppMethod*) = nullptr;
bool(__fastcall* o_RenderSettings_get_fog)(const Il2CppMethod*) = nullptr;
float(__fastcall* o_RenderSettings_get_fogStartDistance)(const Il2CppMethod*) = nullptr;
float(__fastcall* o_RenderSettings_get_fogEndDistance)(const Il2CppMethod*) = nullptr;
void(__fastcall* o_RenderSettings_get_fogColor_Injected)(Color*, const Il2CppMethod*) = nullptr;
float(__fastcall* o_RenderSettings_get_fogDensity)(const Il2CppMethod*) = nullptr;
void(__fastcall* o_RenderSettings_set_fog)(bool) = nullptr;
void(__fastcall* o_RenderSettings_set_fogStartDistance)(float) = nullptr;
void(__fastcall* o_RenderSettings_set_fogEndDistance)(float) = nullptr;
void(__fastcall* o_RenderSettings_set_fogColor_Injected)(Color*) = nullptr;
void(__fastcall* o_RenderSettings_set_fogDensity)(float) = nullptr;
uintptr_t(__fastcall* o_RenderSettings_get_skybox)(const Il2CppMethod*) = nullptr;
void(__fastcall* o_RenderSettings_set_skybox)(uintptr_t, const Il2CppMethod*) = nullptr;

static bool FogSetterApiReady() {
    return o_RenderSettings_set_fog && o_RenderSettings_set_fogStartDistance &&
        o_RenderSettings_set_fogEndDistance && o_RenderSettings_set_fogColor_Injected &&
        o_RenderSettings_set_fogDensity;
}

static void CaptureOriginalFog() {
    if (fogOriginalCaptured || !o_RenderSettings_get_fog || !o_RenderSettings_get_fogStartDistance ||
        !o_RenderSettings_get_fogEndDistance || !o_RenderSettings_get_fogColor_Injected ||
        !o_RenderSettings_get_fogDensity) return;
    __try {
        fogOriginalEnabled = o_RenderSettings_get_fog(nullptr);
        fogOriginalStartDistance = o_RenderSettings_get_fogStartDistance(nullptr);
        fogOriginalEndDistance = o_RenderSettings_get_fogEndDistance(nullptr);
        o_RenderSettings_get_fogColor_Injected(&fogOriginalColor, nullptr);
        fogOriginalDensity = o_RenderSettings_get_fogDensity(nullptr);
        fogOriginalCaptured = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        fogOriginalCaptured = false;
    }
}

static void ApplyFogSettings() {
    if (!fogEnabled || !FogSetterApiReady()) {
        if (fogEnabled) strcpy_s(fogStatus, "Unity fog setter API unavailable");
        return;
    }
    CaptureOriginalFog();
    Color color(fogColor[0], fogColor[1], fogColor[2], 1.0f);
    __try {
        o_RenderSettings_set_fog(true);
        o_RenderSettings_set_fogStartDistance(fogStartDistance);
        o_RenderSettings_set_fogEndDistance(fogEndDistance);
        o_RenderSettings_set_fogColor_Injected(&color);
        o_RenderSettings_set_fogDensity(fogDensity);
        InterlockedIncrement(&fogApplyCalls);
        strcpy_s(fogStatus, "Applied through Unity setters");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(fogStatus, "Unity fog setter call failed");
    }
}

static void RestoreOriginalFog() {
    if (!fogOriginalCaptured || !FogSetterApiReady()) {
        strcpy_s(fogStatus, fogOriginalCaptured ? "Unity fog setter API unavailable" : "Disabled; no captured state");
        return;
    }
    __try {
        o_RenderSettings_set_fogStartDistance(fogOriginalStartDistance);
        o_RenderSettings_set_fogEndDistance(fogOriginalEndDistance);
        o_RenderSettings_set_fogColor_Injected(&fogOriginalColor);
        o_RenderSettings_set_fogDensity(fogOriginalDensity);
        o_RenderSettings_set_fog(fogOriginalEnabled);
        InterlockedIncrement(&fogRestoreCalls);
        fogOriginalCaptured = false;
        strcpy_s(fogStatus, "Original fog restored");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(fogStatus, "Fog restore failed");
    }
}

Vector3(__fastcall* o_WorldToScreenPoint)(uintptr_t, Vector3) = nullptr;

Matrix16(__fastcall* o_Camera_get_worldToCameraMatrix)(uintptr_t) = nullptr;

Matrix16(__fastcall* o_Camera_get_projectionMatrix)(uintptr_t) = nullptr;

short(__fastcall* o_GunController_GetCurrentAmmo)(uintptr_t) = nullptr;
void(__fastcall* o_Gloves_SetArms)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_ArmsLod_SetVisible)(uintptr_t, bool, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Weaponry_TakeWeapon)(uintptr_t, uint8_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_GunController_Fire)(uintptr_t, Vector3, const Il2CppMethod*) = nullptr;
void(__fastcall* o_GunController_Command)(uintptr_t, uintptr_t, float, float, const Il2CppMethod*) = nullptr;
HitCasterCer(__fastcall* o_HitCaster_Cast)(Vector3, Vector3, float,
    HitCasterCeq, const Il2CppMethod*) = nullptr;
thread_local bool insideLocalGunFire = false;
thread_local bool insideAutoFireRequest = false;
thread_local int activeLocalHitCastDepth = 0;
thread_local HitCasterCeq aimbotLastCeq = {};
thread_local bool aimbotLastCeqValid = false;
thread_local uintptr_t aimbotLastCeqGun = 0;
thread_local const Il2CppMethod* aimbotLastHitCasterMethod = nullptr;
void(__fastcall* o_AimView_Awake)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_AimView_UpdateSniperPanels)(uintptr_t, float, float, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Component_get_gameObject)(uintptr_t) = nullptr;
bool(__fastcall* o_Object_IsAlive)(uintptr_t, const Il2CppMethod*) = nullptr;
bool(__fastcall* o_Physics_RaycastHit)(Vector3, Vector3, RaycastHitNative*, float, int, int, const Il2CppMethod*) = nullptr;
Il2CppArray*(__fastcall* o_Physics_RaycastAll)(Vector3, Vector3, float, int, int, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_RaycastHit_get_collider)(RaycastHitNative*, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Component_GetInParent)(uintptr_t, uintptr_t, bool, const Il2CppMethod*) = nullptr;
Il2CppString*(__fastcall* o_Component_get_tag)(uintptr_t, const Il2CppMethod*) = nullptr;
int(__fastcall* o_SurfaceType_FromTag)(Il2CppString*, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Component_GetInChildren)(uintptr_t, uintptr_t, bool, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_Transform_get_parent)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_GameObject_SetActive)(uintptr_t, bool) = nullptr;
bool(__fastcall* o_GameObject_get_activeInHierarchy)(uintptr_t) = nullptr;
void(__fastcall* o_CanvasGroup_set_alpha)(uintptr_t, float) = nullptr;
void(__fastcall* o_HUDView_Update)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_GameChatHud_SendMessage)(uintptr_t, Il2CppString*, const Il2CppMethod*) = nullptr;
void(__fastcall* o_HitMarkerView_Show)(uintptr_t, bool, bool, const Il2CppMethod*) = nullptr;
void(__fastcall* o_HitMarkerView_LocalHit)(uintptr_t, void*, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_PlayerController_Command)(uintptr_t, uintptr_t, float, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_KeyboardControl_BuildCommand)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_PlayerControls_Update)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_CameraMovementController_Update)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_CameraMovementController_FixedUpdate)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_AimController_ApplySnapshot)(uintptr_t, uintptr_t, const Il2CppMethod*) = nullptr;
uintptr_t(__fastcall* o_AimController_GetSnapshot)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_AimController_SetHeadDirective)(uintptr_t, Vector3, const Il2CppMethod*) = nullptr;
Vector3(__fastcall* o_AimController_GetHeadDirective)(uintptr_t, const Il2CppMethod*) = nullptr;
void(__fastcall* o_Camera_FireOnPreCull)(uintptr_t, const Il2CppMethod*) = nullptr;
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

static void SetRendererMaterialPair(uintptr_t renderer, uintptr_t first, uintptr_t second)
{
    if (!renderer || !first || !second || !o_Renderer_set_materials || !g_il2cpp.array_new) return;
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) return;
    Il2CppArray* array = g_il2cpp.array_new(materialClass, 2);
    const uintptr_t arrayAddress = reinterpret_cast<uintptr_t>(array);
    if (!arrayAddress) return;
    *reinterpret_cast<uintptr_t*>(arrayAddress + 0x20) = first;
    *reinterpret_cast<uintptr_t*>(arrayAddress + 0x20 + sizeof(uintptr_t)) = second;
    o_Renderer_set_materials(renderer, array);
}

static uintptr_t GetCurrentLocalWeaponController();
static uintptr_t GetCurrentLocalCharacterLodGroup();

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
    for (WeaponChamsRenderer& entry : weaponChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (ArmChamsRenderer& entry : armChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (GloveChamsRenderer& entry : gloveChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (LocalPlayerChamsRenderer& entry : localPlayerChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    for (EnemyChamsRenderer& entry : enemyChamsRenderers) if (entry.renderer) excluded[entry.renderer] = true;
    const uintptr_t localCharacterLod = GetCurrentLocalCharacterLodGroup();
    if (localCharacterLod) {
        const uintptr_t localBodyRenderer = *reinterpret_cast<uintptr_t*>(localCharacterLod + 0x60);
        if (localBodyRenderer) excluded[localBodyRenderer] = true;
    }
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
            { "OfflineModeController", 0x298 },
            { "GrenadeBattleWithBotsController", 0x2E0 },
            { "FreeForAllWithBotsController", 0x2B8 },
            { "DefuseWithBotsController", 0x2F8 },
            { "EscalationWithBotsController", 0x358 },
            { "DeathmatchWithBotsController", 0x2C0 },
            { "ArmsRaceWithBotsController", 0x2E8 },
            { "DuelV2WithBotsController", 0x298 },
            { "RankedDefuseController", 0x328 }
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
        o_CheatRuntime_SetThirdPerson(runtime, thirdPersonEnabled, nullptr);
        strcpy_s(thirdPersonStatus, thirdPersonEnabled ?
            "Active; custom built-in TPS camera" : "Disabled; original offsets restored");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(thirdPersonStatus, "Native TPS transition failed");
        return false;
    }
}

static bool ApplyAdminBhopState()
{
    const uintptr_t player = liveHudLocalPlayer;
    if (!player) { strcpy_s(adminBhopStatus, "Waiting for local player"); return false; }
    __try {
        const uintptr_t movement = *reinterpret_cast<uintptr_t*>(player + 0xE0);
        const uintptr_t translation = movement ? *reinterpret_cast<uintptr_t*>(movement + 0x78) : 0;
        const uintptr_t jump = translation ? *reinterpret_cast<uintptr_t*>(translation + 0x38) : 0;
        if (!movement || !jump) { strcpy_s(adminBhopStatus, "Waiting for native movement parameters"); return false; }
        if (movement != adminBhopObservedMovement || jump != adminBhopObservedJumpParameters) {
            adminBhopObservedMovement = movement;
            adminBhopObservedJumpParameters = jump;
            adminBhopOriginalEnabled = *reinterpret_cast<bool*>(jump + 0x10);
            adminBhopOriginalSpeedMultiplier = *reinterpret_cast<float*>(jump + 0x14);
            adminBhopOriginalMaxSpeed = *reinterpret_cast<float*>(jump + 0x1C);
            adminBhopOriginalCaptured = true;
        }
        const uintptr_t runtime = GetNativeCheatRuntime();
        // Never toggle the native BHop state during Pixel Surf: doing that in
        // mid-air mutates native movement state and can reverse the trajectory.
        // Pixel Surf isolation happens only at the command-input layer below.
        if (runtime && o_CheatRuntime_SetBhop)
            o_CheatRuntime_SetBhop(runtime, movement, adminBhopEnabled);
        *reinterpret_cast<bool*>(jump + 0x10) = adminBhopEnabled;
        if (adminBhopEnabled) {
            // Keep native propulsion continuous. Toggling this multiplier to
            // zero on brief A/D gaps caused the visible stop-then-resume jerk and
            // killed backward momentum after releasing S.
            const float scaledMultiplier = adminBhopSpeedMultiplier *
                (adminBhopMaxSpeed / 6.50f);
            *reinterpret_cast<float*>(jump + 0x14) = scaledMultiplier;
            *reinterpret_cast<float*>(jump + 0x1C) = adminBhopMaxSpeed;
            strcpy_s(adminBhopStatus, adminBhopCsStrafeMode ?
                "Native admin BHop active; CS air strafe" :
                (runtime ? "Native admin BHop active" :
                    "Native jump BHop active; runtime pending"));
        } else if (adminBhopOriginalCaptured) {
            *reinterpret_cast<bool*>(jump + 0x10) = adminBhopOriginalEnabled;
            *reinterpret_cast<float*>(jump + 0x14) = adminBhopOriginalSpeedMultiplier;
            *reinterpret_cast<float*>(jump + 0x1C) = adminBhopOriginalMaxSpeed;
            strcpy_s(adminBhopStatus, "Disabled; native values restored");
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { strcpy_s(adminBhopStatus, "Native BHop apply failed"); return false; }
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

static uintptr_t CreateWeaponChamsMaterial(const char* shaderName, bool glass);
static void ConfigureOutlineChamsMaterial(uintptr_t material);
static bool CreateOutlineCloneUnsafe(uintptr_t renderer, uintptr_t material,
    uintptr_t* outlineObject, uintptr_t* outlineRenderer);
static void DestroyOutlineCloneUnsafe(uintptr_t* outlineObject, uintptr_t* outlineRenderer);

using t_RagdollActivate = void(__fastcall*)(uintptr_t, uintptr_t, Vector3, bool,
    void*, uintptr_t, bool, bool, const Il2CppMethod*);
using t_RagdollManagerRelease = void(__fastcall*)(uintptr_t, uintptr_t, const Il2CppMethod*);
t_RagdollActivate o_RagdollActivate = nullptr;
t_RagdollManagerRelease o_RagdollManagerRelease = nullptr;

static void SetCorpseRigidbodiesFrozen(FrozenCorpseEntry& corpse, bool frozen)
{
    if (!base) return;
    using GetKinematicFn = bool(__fastcall*)(uintptr_t, const Il2CppMethod*);
    using SetKinematicFn = void(__fastcall*)(uintptr_t, bool, const Il2CppMethod*);
    using SetVectorFn = void(__fastcall*)(uintptr_t, Vector3, const Il2CppMethod*);
    const auto getKinematic = reinterpret_cast<GetKinematicFn>(base + OFFSET_RIGIDBODY_GET_IS_KINEMATIC);
    const auto setKinematic = reinterpret_cast<SetKinematicFn>(base + OFFSET_RIGIDBODY_SET_IS_KINEMATIC);
    const auto setVelocity = reinterpret_cast<SetVectorFn>(base + OFFSET_RIGIDBODY_SET_VELOCITY);
    const auto setAngularVelocity = reinterpret_cast<SetVectorFn>(base + OFFSET_RIGIDBODY_SET_ANGULAR_VELOCITY);
    __try {
        if (frozen && corpse.rigidbodies.empty()) {
            const uintptr_t bodies = *reinterpret_cast<uintptr_t*>(corpse.ragdoll + 0x40);
            const int count = bodies ? *reinterpret_cast<int*>(bodies + 0x18) : 0;
            const uintptr_t items = bodies ? *reinterpret_cast<uintptr_t*>(bodies + 0x10) : 0;
            if (!items || count <= 0 || count > 128) return;
            corpse.rigidbodies.reserve(count);
            for (int i = 0; i < count; ++i) {
                const uintptr_t body = *reinterpret_cast<uintptr_t*>(items + 0x20 +
                    static_cast<uintptr_t>(i) * sizeof(uintptr_t));
                if (!body) continue;
                const bool wasKinematic = getKinematic(body, nullptr);
                const Vector3 zero(0.0f, 0.0f, 0.0f);
                setVelocity(body, zero, nullptr);
                setAngularVelocity(body, zero, nullptr);
                setKinematic(body, true, nullptr);
                corpse.rigidbodies.push_back({ body, wasKinematic });
            }
        }
        else if (!frozen) {
            for (const auto& body : corpse.rigidbodies)
                if (body.first) setKinematic(body.first, body.second, nullptr);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uintptr_t CreateFrozenCorpseMaterial(uintptr_t originalMaterial, int mode)
{
    if (!g_il2cpp.object_new) return 0;
    Il2CppClass* materialClass = g_il2cpp.find_class("UnityEngine", "Material");
    if (!materialClass) return 0;
    if (mode == 0 && originalMaterial && o_Material_copy_ctor) {
        const uintptr_t material = reinterpret_cast<uintptr_t>(g_il2cpp.object_new(materialClass));
        if (!material) return 0;
        o_Material_copy_ctor(material, originalMaterial, nullptr);
        // Preserve the original shader/texture: only switch its common blend properties
        // for alpha fading. Unsupported properties are harmless on custom shaders.
        if (o_Material_SetFloat && g_il2cpp.string_new) {
            o_Material_SetFloat(material, g_il2cpp.string_new("_Mode"), 3.0f);
            o_Material_SetFloat(material, g_il2cpp.string_new("_Surface"), 1.0f);
            o_Material_SetFloat(material, g_il2cpp.string_new("_SrcBlend"), 5.0f);
            o_Material_SetFloat(material, g_il2cpp.string_new("_DstBlend"), 10.0f);
            o_Material_SetFloat(material, g_il2cpp.string_new("_ZWrite"), 0.0f);
        }
        if (o_Material_set_renderQueue) o_Material_set_renderQueue(material, 3000);
        return material;
    }
    static const char* shaders[8][4] = {
        { nullptr, nullptr, nullptr, nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Standard", "Legacy Shaders/Transparent/VertexLit", nullptr },
        { "Standard", "Legacy Shaders/Transparent/Specular", "Legacy Shaders/Specular", nullptr },
        { "Unlit/Transparent", "Sprites/Default", "Legacy Shaders/Transparent/Diffuse", nullptr },
        { "Unlit/Transparent", "Sprites/Default", "Legacy Shaders/Transparent/Diffuse", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr }
    };
    if (mode < 1 || mode > 7) return 0;
    for (const char* shader : shaders[mode]) {
        if (!shader) break;
        const uintptr_t material = CreateWeaponChamsMaterial(shader, true);
        if (material) {
            if (mode == 7) ConfigureOutlineChamsMaterial(material);
            return material;
        }
    }
    return 0;
}

static void SetupFrozenCorpseMaterial(FrozenCorpseEntry& corpse)
{
    if (!corpse.ragdoll || !o_Renderer_get_material || !o_Renderer_set_material ||
        !o_Material_set_color) return;
    __try {
        const uintptr_t lodGroup = *reinterpret_cast<uintptr_t*>(corpse.ragdoll + 0x90);
        corpse.renderer = lodGroup ? *reinterpret_cast<uintptr_t*>(lodGroup + 0x60) : 0;
        corpse.originalMaterial = corpse.renderer ? o_Renderer_get_material(corpse.renderer) : 0;
        if (!corpse.renderer || !corpse.originalMaterial) return;
        corpse.materialMode = freezeCorpsesChamsMode;
        corpse.fadeMaterial = CreateFrozenCorpseMaterial(corpse.originalMaterial, corpse.materialMode);
        if (!corpse.fadeMaterial) return;
        if (g_il2cpp.gchandle_new)
            corpse.fadeMaterialHandle = g_il2cpp.gchandle_new(
                reinterpret_cast<Il2CppObject*>(corpse.fadeMaterial), false);
        if (corpse.materialMode == 7) {
            o_Renderer_set_material(corpse.renderer, corpse.originalMaterial);
            CreateOutlineCloneUnsafe(corpse.renderer, corpse.fadeMaterial,
                &corpse.outlineObject, &corpse.outlineRenderer);
        }
        else o_Renderer_set_material(corpse.renderer, corpse.fadeMaterial);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void SetCorpseRendererVisibleUnsafe(uintptr_t renderer, bool visible);

static void RestoreFrozenCorpseMaterial(FrozenCorpseEntry& corpse)
{
    SetCorpseRendererVisibleUnsafe(corpse.renderer, true);
    DestroyOutlineCloneUnsafe(&corpse.outlineObject, &corpse.outlineRenderer);
    __try {
        if (corpse.renderer && corpse.originalMaterial && o_Renderer_set_material)
            o_Renderer_set_material(corpse.renderer, corpse.originalMaterial);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (corpse.fadeMaterialHandle && g_il2cpp.gchandle_free)
        g_il2cpp.gchandle_free(corpse.fadeMaterialHandle);
    corpse.fadeMaterialHandle = 0;
    corpse.fadeMaterial = 0;
}

static uintptr_t GetCorpseRendererUnsafe(uintptr_t ragdoll)
{
    __try {
        const uintptr_t lodGroup = ragdoll ? *reinterpret_cast<uintptr_t*>(ragdoll + 0x90) : 0;
        return lodGroup ? *reinterpret_cast<uintptr_t*>(lodGroup + 0x60) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static void SetCorpseRendererVisibleUnsafe(uintptr_t renderer, bool visible)
{
    __try { if (renderer && o_Renderer_set_enabled) o_Renderer_set_enabled(renderer, visible); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

void __fastcall hk_RagdollActivate(uintptr_t ragdoll, uintptr_t biped, Vector3 velocity,
    bool c, void* skinName, uintptr_t shotData, bool f, bool g, const Il2CppMethod* method)
{
    o_RagdollActivate(ragdoll, biped, velocity, c, skinName, shotData, f, g, method);
    if (!keyValidated || !freezeCorpsesEnabled || !ragdoll) return;
    FrozenCorpseEntry corpse{};
    corpse.ragdoll = ragdoll;
    corpse.manager = 0;
    corpse.createdAt = GetTickCount64();
    corpse.releaseAt = corpse.createdAt +
        static_cast<ULONGLONG>(freezeCorpsesDuration * 1000.0f);
    corpse.releaseRequested = false;
    corpse.renderer = 0;
    corpse.originalMaterial = 0;
    corpse.fadeMaterial = 0;
    corpse.fadeMaterialHandle = 0;
    corpse.outlineObject = 0;
    corpse.outlineRenderer = 0;
    corpse.materialMode = freezeCorpsesChamsMode;
    SetCorpseRigidbodiesFrozen(corpse, true);
    SetupFrozenCorpseMaterial(corpse);
    AcquireSRWLockExclusive(&frozenCorpseLock);
    for (auto it = frozenCorpses.begin(); it != frozenCorpses.end();) {
        if (it->ragdoll != ragdoll) { ++it; continue; }
        RestoreFrozenCorpseMaterial(*it);
        SetCorpseRigidbodiesFrozen(*it, false);
        it = frozenCorpses.erase(it);
    }
    frozenCorpses.push_back(std::move(corpse));
    ReleaseSRWLockExclusive(&frozenCorpseLock);
}

void __fastcall hk_RagdollManagerRelease(uintptr_t manager, uintptr_t ragdoll,
    const Il2CppMethod* method)
{
    if (keyValidated && freezeCorpsesEnabled && ragdoll) {
        const ULONGLONG now = GetTickCount64();
        AcquireSRWLockExclusive(&frozenCorpseLock);
        for (FrozenCorpseEntry& corpse : frozenCorpses) {
            if (corpse.ragdoll != ragdoll || now >= corpse.releaseAt) continue;
            corpse.manager = manager;
            corpse.releaseRequested = true;
            ReleaseSRWLockExclusive(&frozenCorpseLock);
            return;
        }
        ReleaseSRWLockExclusive(&frozenCorpseLock);
    }
    o_RagdollManagerRelease(manager, ragdoll, method);
}

static void ApplyFrozenCorpseFadeUnsafe(uintptr_t material, uintptr_t renderer,
    int mode, float alpha, ULONGLONG now)
{
    __try {
        float r = freezeCorpsesChamsColor[0], g = freezeCorpsesChamsColor[1],
            b = freezeCorpsesChamsColor[2];
        const float t = static_cast<float>(now % 60000) * 0.001f * freezeCorpsesAnimationSpeed;
        if (mode == 5) {
            r = 0.5f + 0.5f * sinf(t);
            g = 0.5f + 0.5f * sinf(t + 2.0943951f);
            b = 0.5f + 0.5f * sinf(t + 4.1887902f);
        }
        else if (mode == 6) {
            const float intensity = 0.25f + 0.75f * (0.5f + 0.5f * sinf(t));
            r *= intensity; g *= intensity; b *= intensity;
        }
        if (mode == 4 && o_Material_SetFloat && g_il2cpp.string_new) {
            o_Material_SetFloat(material, g_il2cpp.string_new("_Metallic"), freezeCorpsesMetallic);
            o_Material_SetFloat(material, g_il2cpp.string_new("_Glossiness"), freezeCorpsesSmoothness);
        }
        if (mode == 0 && o_Material_get_color) {
            const Color original = o_Material_get_color(material);
            r = original.r; g = original.g; b = original.b;
        }
        o_Material_set_color(material, Color(r, g, b, alpha));
        if (mode != 7 && renderer && o_Renderer_get_material && o_Renderer_set_material &&
            o_Renderer_get_material(renderer) != material)
            o_Renderer_set_material(renderer, material);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void UpdateFrozenCorpses()
{
    const ULONGLONG now = GetTickCount64();
    std::vector<std::pair<uintptr_t, uintptr_t>> releases;
    AcquireSRWLockExclusive(&frozenCorpseLock);
    for (auto it = frozenCorpses.begin(); it != frozenCorpses.end();) {
        if (freezeCorpsesEnabled && now < it->releaseAt) {
            if (it->fadeMaterial && o_Material_set_color) {
                float alpha = it->materialMode == 2 ? freezeCorpsesChamsAlpha : 1.0f;
                if (freezeCorpsesFadeEnabled) {
                    float fadeFactor = 1.0f;
                    const ULONGLONG fadeMs = static_cast<ULONGLONG>(
                        fmaxf(0.05f, freezeCorpsesFadeDuration) * 1000.0f);
                    const ULONGLONG remaining = it->releaseAt - now;
                    if (remaining < fadeMs)
                        fadeFactor = static_cast<float>(remaining) / static_cast<float>(fadeMs);
                    alpha *= fadeFactor;
                }
                ApplyFrozenCorpseFadeUnsafe(it->fadeMaterial, it->renderer,
                    it->materialMode, alpha, now);
            }
            ++it;
            continue;
        }
        RestoreFrozenCorpseMaterial(*it);
        SetCorpseRigidbodiesFrozen(*it, false);
        if (it->releaseRequested && it->manager && it->ragdoll)
            releases.push_back({ it->manager, it->ragdoll });
        it = frozenCorpses.erase(it);
    }
    ReleaseSRWLockExclusive(&frozenCorpseLock);
    // Release outside the lock: the native pool callback can synchronously reuse
    // the ragdoll and enter hk_RagdollActivate again.
    for (const auto& release : releases)
        o_RagdollManagerRelease(release.first, release.second, nullptr);
}

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
        sniperSightObject = instance ? *(uintptr_t*)(instance + 0x50) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { sniperSightObject = 0; }
}

void __fastcall hk_AimView_UpdateSniperPanels(uintptr_t instance, float a, float b, const Il2CppMethod* method)
{
    // This runs on every aiming transition, so it also captures AimView when injected mid-match.
    __try {
        if (instance) {
            liveAimView = instance;
            sniperSightObject = *(uintptr_t*)(instance + 0x50);
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
        const uintptr_t scopeCanvasGroup = *(uintptr_t*)(liveAimView + 0xB0);
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

static float GetCommandAntiAimPitch()
{
    if (silentAntiAimPitch == 1) return 70.0f;  // Down
    if (silentAntiAimPitch == 2) return -70.0f; // Up
    return 0.0f;                                // Neutral
}

static float GetCommandAntiAimYawOffset()
{
    if (silentAntiAimMode == 0) return silentAntiAimYaw;
    if (silentAntiAimMode == 1) {
        silentAntiAimJitterFlip = !silentAntiAimJitterFlip;
        return silentAntiAimYaw +
            (silentAntiAimJitterFlip ? silentAntiAimJitterRange :
                                       -silentAntiAimJitterRange);
    }

    const ULONGLONG now = GetTickCount64();
    if (!silentAntiAimLastSpinTick) silentAntiAimLastSpinTick = now;
    const float dt = static_cast<float>(now - silentAntiAimLastSpinTick) / 1000.0f;
    silentAntiAimLastSpinTick = now;
    silentAntiAimSpinYaw = NormalizeAngle360(
        silentAntiAimSpinYaw + silentAntiAimSpinSpeed * dt);
    return silentAntiAimSpinYaw;
}

static void FixAntiAimMovement(uintptr_t command, float realYaw, float fakeYaw)
{
    if (!command) return;
    __try {
        // dump.cs blr: bzxz +0x10, bzya +0x14.
        const float horizontal = *reinterpret_cast<float*>(command + 0x10);
        const float vertical = *reinterpret_cast<float*>(command + 0x14);
        if (horizontal == 0.0f && vertical == 0.0f) return;
        const float delta = NormalizeAngle180(fakeYaw - realYaw);
        const float radians = delta * 0.017453292519943295f;
        const float cosine = cosf(radians);
        const float sine = sinf(radians);
        float fixedHorizontal = cosine * horizontal - sine * vertical;
        float fixedVertical = sine * horizontal + cosine * vertical;
        const float length = sqrtf(
            fixedHorizontal * fixedHorizontal + fixedVertical * fixedVertical);
        if (length > 1.0f) {
            fixedHorizontal /= length;
            fixedVertical /= length;
        }
        *reinterpret_cast<float*>(command + 0x10) = fixedHorizontal;
        *reinterpret_cast<float*>(command + 0x14) = fixedVertical;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static bool CaptureCommandAntiAimInput(uintptr_t player, uintptr_t command)
{
    if (!silentAntiAimEnabled || !player || !command) return false;
    __try {
        const uintptr_t aimController =
            *reinterpret_cast<uintptr_t*>(player + 0xC8);
        const uintptr_t aimingData = aimController ?
            *reinterpret_cast<uintptr_t*>(aimController + 0x80) : 0;
        if (!aimController || !aimingData) return false;
        silentAntiAimLatestRealAimAngle =
            *reinterpret_cast<Vector3*>(aimingData + 0x18);
        silentAntiAimLatestRealAimEuler =
            *reinterpret_cast<Vector3*>(aimingData + 0x24);
        if (!isfinite(silentAntiAimLatestRealAimAngle.x) ||
            !isfinite(silentAntiAimLatestRealAimAngle.y))
            return false;
        silentAntiAimLatestFiring =
            *reinterpret_cast<bool*>(command + 0x21);
        silentAntiAimLatestPlayer = player;
        silentAntiAimLatestController = aimController;
        silentAntiAimRealCameraAngles = silentAntiAimLatestRealAimAngle;
        silentAntiAimRealCameraAngles.y = NormalizeAngle360(
            silentAntiAimRealCameraAngles.y);
        silentAntiAimRealCameraAngles.z = 0.0f;
        silentAntiAimRealCameraValid = true;
        silentAntiAimLatestInputValid = true;
        liveHudLocalPlayer = player;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        silentAntiAimLatestInputValid = false;
        silentAntiAimRealCameraValid = false;
        return false;
    }
}

static void ApplyCommandAntiAim(uintptr_t player, uintptr_t command)
{
    if (!CaptureCommandAntiAimInput(player, command)) {
        strcpy_s(silentAntiAimStatus, "Waiting for local command/aim data");
        return;
    }
    if (silentAntiAimLatestFiring) {
        strcpy_s(silentAntiAimStatus, "Real angles while firing");
        return;
    }
    __try {
        const float realYaw = NormalizeAngle360(
            silentAntiAimLatestRealAimAngle.y);
        const float yawOffset = GetCommandAntiAimYawOffset();
        const Vector3 fakeAngles(
            GetCommandAntiAimPitch(),
            NormalizeAngle360(realYaw - yawOffset),
            0.0f);
        // dump1 xl: bbb angle mode +0x29 (TargetAngle=0), Vector3 +0x2C.
        // Fake angles exist only in the outgoing command, never in AimingData/camera.
        *reinterpret_cast<uint8_t*>(command + 0x29) = 0;
        *reinterpret_cast<Vector3*>(command + 0x2C) = fakeAngles;
        FixAntiAimMovement(command, realYaw, fakeAngles.y);
        silentAntiAimLatestFakeAngles = fakeAngles;
        InterlockedIncrement(&silentAntiAimAppliedCalls);
        strcpy_s(silentAntiAimStatus,
            silentAntiAimMode == 0 ? "Command anti-aim: Static" :
            (silentAntiAimMode == 1 ? "Command anti-aim: Jitter" :
                                      "Command anti-aim: Spin"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        strcpy_s(silentAntiAimStatus, "Command anti-aim write failed safely");
    }
}

uintptr_t __fastcall hk_KeyboardControl_BuildCommand(
    uintptr_t keyboardControl, uintptr_t player, const Il2CppMethod* method)
{
    const uintptr_t command = o_KeyboardControl_BuildCommand(
        keyboardControl, player, method);
    InterlockedIncrement(&silentAntiAimInputBuildCalls);

    if (keyValidated && adminBhopEnabled && adminBhopCsStrafeMode && !jbActive &&
        player && command && player == liveHudLocalPlayer && lastCharacterController &&
        o_CC_get_isGrounded) {
        __try {
            const bool grounded = o_CC_get_isGrounded(lastCharacterController);
            if (!grounded) {
                float& strafeInput = *reinterpret_cast<float*>(command + 0x10);
                float& forwardInput = *reinterpret_cast<float*>(command + 0x14);
                const float manualStrafe = strafeInput;
                const float originalForwardInput = forwardInput;
                float currentYaw = adminBhopPreviousCameraYaw;
                bool currentYawValid = false;
                const uintptr_t aimController = *reinterpret_cast<uintptr_t*>(player + 0xC8);
                const uintptr_t aimingData = aimController ?
                    *reinterpret_cast<uintptr_t*>(aimController + 0x80) : 0;
                if (aimingData) {
                    currentYaw = reinterpret_cast<Vector3*>(aimingData + 0x18)->y;
                    currentYawValid = isfinite(currentYaw);
                }
                float yawDelta = 0.0f;
                if (currentYawValid && adminBhopCameraYawValid)
                    yawDelta = NormalizeAngle180(currentYaw - adminBhopPreviousCameraYaw);
                if (currentYawValid) {
                    adminBhopPreviousCameraYaw = currentYaw;
                    adminBhopCameraYawValid = true;
                }
                const bool hasManualStrafe = fabsf(manualStrafe) > 0.01f;
                const bool matchingCameraTurn = hasManualStrafe &&
                    fabsf(yawDelta) > 0.001f && manualStrafe * yawDelta > 0.0f;
                InterlockedExchange(&adminBhopManualStrafeHeld,
                    matchingCameraTurn ? 1 : 0);
                if (matchingCameraTurn) {
                    InterlockedExchange64(&adminBhopManualStrafeGraceUntil,
                        static_cast<LONG64>(GetTickCount64()) + 180);
                    forwardInput = fabsf(manualStrafe);
                    strafeInput = 0.0f;
                } else {
                    forwardInput = originalForwardInput < -0.01f ?
                        originalForwardInput : 0.0f;
                    strafeInput = 0.0f;
                }
            } else {
                InterlockedExchange(&adminBhopManualStrafeHeld, 0);
                InterlockedExchange64(&adminBhopManualStrafeGraceUntil, 0);
                adminBhopCameraYawValid = false;
                adminBhopLastManualMotionValid = false;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Keep the builder camera-pure. Fake angles are scoped to the native
    // PlayerController command call below and restored before it returns.
    return command;
}

void __fastcall hk_PlayerController_Command(
    uintptr_t player, uintptr_t command, float deltaTime,
    const Il2CppMethod* method)
{
    const bool localCommand = keyValidated && silentAntiAimEnabled &&
        player && command && (!liveHudLocalPlayer || player == liveHudLocalPlayer);
    if (!localCommand) {
        o_PlayerController_Command(player, command, deltaTime, method);
        return;
    }

    uint8_t originalMode = 0;
    Vector3 originalAngles;
    float originalHorizontal = 0.0f;
    float originalVertical = 0.0f;
    uintptr_t aimingData = 0;
    Vector3 originalAimAngle;
    Vector3 originalEulerAngles;
    bool captured = false;
    __try {
        originalMode = *reinterpret_cast<uint8_t*>(command + 0x29);
        originalAngles = *reinterpret_cast<Vector3*>(command + 0x2C);
        originalHorizontal = *reinterpret_cast<float*>(command + 0x10);
        originalVertical = *reinterpret_cast<float*>(command + 0x14);
        const uintptr_t aimController =
            *reinterpret_cast<uintptr_t*>(player + 0xC8);
        aimingData = aimController ?
            *reinterpret_cast<uintptr_t*>(aimController + 0x80) : 0;
        if (aimingData) {
            originalAimAngle = *reinterpret_cast<Vector3*>(aimingData + 0x18);
            originalEulerAngles = *reinterpret_cast<Vector3*>(aimingData + 0x24);
        }
        captured = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { captured = false; }

    if (captured) ApplyCommandAntiAim(player, command);
    o_PlayerController_Command(player, command, deltaTime, method);

    // xl is reused. Restore command and complete local view vectors synchronously,
    // before any camera update can observe command-side pose state.
    if (captured) __try {
        *reinterpret_cast<uint8_t*>(command + 0x29) = originalMode;
        *reinterpret_cast<Vector3*>(command + 0x2C) = originalAngles;
        *reinterpret_cast<float*>(command + 0x10) = originalHorizontal;
        *reinterpret_cast<float*>(command + 0x14) = originalVertical;
        if (aimingData) {
            *reinterpret_cast<Vector3*>(aimingData + 0x18) = originalAimAngle;
            *reinterpret_cast<Vector3*>(aimingData + 0x24) = originalEulerAngles;
        }
        InterlockedIncrement(&silentAntiAimCommandCalls);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static bool ReadAntiAimFpsCameraTransformUnsafe(uintptr_t* transform)
{
    if (!transform) return false;
    *transform = 0;
    const uintptr_t player = silentAntiAimLatestPlayer;
    if (!player) return false;
    __try {
        // Source m_aim->m_FPSP maps exactly to dump.cs:
        // AimController.FPSCamera (+0x68).
        const uintptr_t aimController =
            *reinterpret_cast<uintptr_t*>(player + 0xC8);
        if (!aimController || aimController != silentAntiAimLatestController)
            return false;
        *transform = *reinterpret_cast<uintptr_t*>(aimController + 0x68);
        return *transform != 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *transform = 0;
        return false;
    }
}

static bool ReadAntiAimTpsCameraTransformUnsafe(uintptr_t* transform)
{
    if (!transform) return false;
    *transform = 0;
    const uintptr_t player = silentAntiAimLatestPlayer;
    if (!player) return false;
    __try {
        // Source mainCamera->getTransform maps to PlayerController.dcut (+0x80)
        // and PlayerMainCamera.dcvd / backing Transform bzza (+0x38).
        const uintptr_t mainCamera =
            *reinterpret_cast<uintptr_t*>(player + 0x80);
        if (!mainCamera) return false;
        *transform = *reinterpret_cast<uintptr_t*>(mainCamera + 0x38);
        return *transform != 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *transform = 0;
        return false;
    }
}

static void RestoreAntiAimCameraTransformUnsafe(uintptr_t transform)
{
    if (!transform || !silentAntiAimEnabled ||
        !silentAntiAimLatestInputValid || !silentAntiAimRealCameraValid ||
        !o_Transform_set_eulerAngles)
        return;
    __try {
        o_Transform_set_eulerAngles(transform, silentAntiAimRealCameraAngles);
        InterlockedIncrement(&silentAntiAimCameraRestoreCalls);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uintptr_t GetAuthoritativeLocalWeaponController();
static uintptr_t GetCurrentLocalWeaponController();
static void UpdateWeaponChams(uintptr_t knownWeaponController = 0);
static void UpdateArmChams(uintptr_t knownArmsLodGroup = 0);
static void UpdateGloveChams(uintptr_t knownArmsLodGroup = 0);
static void UpdateLocalPlayerChams(uintptr_t knownCharacterLodGroup = 0);
static void UpdateEnemyChams();
static void UpdatePenetrableSurfaceVisualization();

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
        const uintptr_t aimingData = aimController ? *reinterpret_cast<uintptr_t*>(aimController + 0x80) : 0;
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

static bool SendPendingPixelSurfChatMessageUnsafe()
{
    if (!o_GameChatHud_SendMessage || !g_il2cpp.string_new) return false;
    const uintptr_t controller = GetActiveGameController();
    if (!controller) return false;
    __try {
        // GameController.<cewd>k__BackingField is the live GameChatHud at +0xF8.
        const uintptr_t chatHud = *reinterpret_cast<uintptr_t*>(controller + 0xF8);
        if (!chatHud) return false;
        Il2CppString* message = g_il2cpp.string_new("ze0nware|pixelsurfed");
        if (!message) return false;
        o_GameChatHud_SendMessage(chatHud, message, nullptr);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void __fastcall hk_HUDView_Update(uintptr_t instance, const Il2CppMethod* method)
{
    __try {
        liveAimView = instance ? *(uintptr_t*)(instance + 0x38) : 0;
        liveHitMarkerView = instance ? *(uintptr_t*)(instance + 0x48) : 0;
        liveHudLocalPlayer = instance ? *(uintptr_t*)(instance + 0xD0) : 0;
        sniperSightObject = liveAimView ? *(uintptr_t*)(liveAimView + 0x50) : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { liveAimView = 0; liveHitMarkerView = 0; liveHudLocalPlayer = 0; sniperSightObject = 0; }
    o_HUDView_Update(instance, method);
    if (InterlockedExchange(&pendingPixelSurfChatMessage, 0))
        SendPendingPixelSurfChatMessageUnsafe();
    UpdateFrozenCorpses();
    UpdatePenetrableSurfaceVisualization();
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

static void ReadLocalHitPayload(uintptr_t shotData, int& damage, char* hitbox, size_t hitboxSize)
{
    damage = -1;
    if (!hitbox || hitboxSize < 8) return;
    strcpy_s(hitbox, hitboxSize, "Unknown");
    if (!shotData) return;
    __try {
        const uintptr_t hits = *reinterpret_cast<uintptr_t*>(shotData + 0x38); // chv.cdwv: chs[]
        const uintptr_t count = hits ? *reinterpret_cast<uintptr_t*>(hits + 0x18) : 0;
        int damageSum = 0, head = 0, body = 0, legs = 0;
        if (count > 0 && count <= 64) {
            for (uintptr_t i = 0; i < count; ++i) {
                const uintptr_t hit = *reinterpret_cast<uintptr_t*>(hits + 0x20 + i * sizeof(uintptr_t));
                if (!hit) continue;
                const int hitDamage = *reinterpret_cast<int*>(hit + 0x2C); // chs.cdvw
                if (hitDamage > 0 && hitDamage <= 500) damageSum += hitDamage;
                const int bone = *reinterpret_cast<int*>(hit + 0x34); // chs.cdvy: BipedMap.blh
                if (bone == 0 || bone == 1) ++head;
                else if (bone >= 13 && bone <= 18) ++legs;
                else if ((bone >= 2 && bone <= 12) || bone == 19) ++body;
            }
        }
        if (damageSum > 0 && damageSum <= 500) damage = damageSum;
        else {
            const int total = *reinterpret_cast<int*>(shotData + 0x2C); // chv.cdwt fallback
            if (total > 0 && total <= 500) damage = total;
        }
        if (head >= body && head >= legs && head > 0) strcpy_s(hitbox, hitboxSize, "Head");
        else if (body >= legs && body > 0) strcpy_s(hitbox, hitboxSize, "Body");
        else if (legs > 0) strcpy_s(hitbox, hitboxSize, "Legs");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        damage = -1;
        strcpy_s(hitbox, hitboxSize, "Unknown");
    }
}

static bool IsPointOnBulletCast(const Vector3& point, const Vector3& start,
    const Vector3& direction, float maxDistance)
{
    if (!isfinite(point.x) || !isfinite(point.y) || !isfinite(point.z) ||
        !isfinite(maxDistance) || maxDistance <= 0.0f) return false;
    const float directionLength = sqrtf(direction.x * direction.x +
        direction.y * direction.y + direction.z * direction.z);
    if (!isfinite(directionLength) || directionLength <= 0.0001f) return false;
    const float dx = point.x - start.x;
    const float dy = point.y - start.y;
    const float dz = point.z - start.z;
    const float nx = direction.x / directionLength;
    const float ny = direction.y / directionLength;
    const float nz = direction.z / directionLength;
    const float along = dx * nx + dy * ny + dz * nz;
    if (along < -0.05f || along > maxDistance + 1.0f) return false;
    const float distanceSquared = dx * dx + dy * dy + dz * dz;
    const float perpendicularSquared = distanceSquared - along * along;
    // Real penetration entry/exit points lie directly on this shot ray. Unused
    // cjv fields are zero-filled and otherwise produced a cube at the map origin.
    return perpendicularSquared <= 0.0625f;
}

static void AddBulletImpact(const Vector3& position, bool serverConfirmed)
{
    if (!isfinite(position.x) || !isfinite(position.y) || !isfinite(position.z)) return;
    const ULONGLONG now = GetTickCount64();
    AcquireSRWLockExclusive(&bulletImpactLock);
    // A server-confirmed marker supersedes an almost identical recent blue one,
    // avoiding duplicates when the hit callback is repeated for one shot.
    bool duplicate = false;
    for (auto it = bulletImpacts.rbegin(); it != bulletImpacts.rend(); ++it) {
        if (now - it->createdAt > 250) break;
        if (it->serverConfirmed == serverConfirmed &&
            it->position.Distance(position) < 0.025f) {
            duplicate = true;
            break;
        }
    }
    if (!duplicate) bulletImpacts.push_back({ position, now, serverConfirmed });
    if (bulletImpacts.size() > 128)
        bulletImpacts.erase(bulletImpacts.begin(), bulletImpacts.begin() + (bulletImpacts.size() - 128));
    ReleaseSRWLockExclusive(&bulletImpactLock);
}

void __fastcall hk_HitMarkerView_LocalHit(uintptr_t instance, void* victim,
    uintptr_t shotData, const Il2CppMethod* method)
{
    // bbcz supplies the victim PlayerController directly. Accept it only when it is on
    // the active local HUD and follows an actual local GunController -> HitCaster cast.
    if (keyValidated && bulletImpactsEnabled && instance && instance == liveHitMarkerView && victim && shotData && activeLocalHitCastDepth > 0) {
        Vector3 confirmedImpact, castEndFallback;
        if (ResolveConfirmedPlayerImpact(confirmedImpact, castEndFallback))
            AddBulletImpact(confirmedImpact, true);
    }
    if (keyValidated && hitLogEnabled && instance && instance == liveHitMarkerView && victim && shotData) {
        const ULONGLONG now = GetTickCount64();
        void* localPlayer = GetLocalPC();
        const unsigned char localTeam = GetPCTeam(localPlayer);
        const unsigned char victimTeam = GetPCTeam(victim);
        // bbcz is invoked synchronously from inside the original HitCaster::Cast. The
        // thread-local depth is set before entering Cast, so no remote/network callback
        // on another thread can pass this gate and no post-return timestamp race exists.
        const bool insideThisLocalCast = activeLocalHitCastDepth > 0;
        if (insideThisLocalCast && localPlayer && victim != localPlayer &&
            localTeam && localTeam != 3 && victimTeam && victimTeam != 3 && victimTeam != localTeam &&
            (shotData != hitLogLastResult || now - hitLogLastResultAt > 250)) {
            HitLogEntry entry = {};
            ReadLocalHitPayload(shotData, entry.damage, entry.hitbox, sizeof(entry.hitbox));
            if (entry.damage > 0) {
                GetPCName(victim, entry.enemyName, sizeof(entry.enemyName));
                entry.createdAt = now;
                AcquireSRWLockExclusive(&hitLogLock);
                hitLogEntries.push_back(entry);
                if (hitLogEntries.size() > 6)
                    hitLogEntries.erase(hitLogEntries.begin(), hitLogEntries.begin() + (hitLogEntries.size() - 6));
                hitLogLastResult = shotData;
                hitLogLastResultAt = now;
                ReleaseSRWLockExclusive(&hitLogLock);
            }
        }
    }
    o_HitMarkerView_LocalHit(instance, victim, shotData, method);
}

void __fastcall hk_HitMarkerView_Show(uintptr_t instance, bool value, bool playSound, const Il2CppMethod* method)
{
    // HitMarkerView.bbcx is the game's confirmed marker-display path. Restrict the
    // overlay to the marker owned by the active local HUD; remote HUD instances and
    // raw world/surface casts cannot trigger it.
    if (keyValidated && value && hitMarkerEnabled && instance && instance == liveHitMarkerView) {
        const ULONGLONG now = GetTickCount64();
        {
            InterlockedIncrement(&hitMarkerCalls);
            Vector3 impactPoint, castEndFallback;
            const bool resolved = ResolveConfirmedPlayerImpact(impactPoint, castEndFallback);
            bool haveWorldFallback = false;
            AcquireSRWLockShared(&hitMarkerLock);
            haveWorldFallback = latestHitMarkerCastValid && latestHitMarkerCastAt &&
                now >= latestHitMarkerCastAt && now - latestHitMarkerCastAt <= 2000;
            ReleaseSRWLockShared(&hitMarkerLock);

            AcquireSRWLockExclusive(&hitMarkerLock);
            hitMarkers.push_back({ resolved ? impactPoint : castEndFallback, now,
                !resolved && !haveWorldFallback });
            if (hitMarkers.size() > 64)
                hitMarkers.erase(hitMarkers.begin(), hitMarkers.begin() + (hitMarkers.size() - 64));
            ReleaseSRWLockExclusive(&hitMarkerLock);
            InterlockedIncrement(resolved ? &hitMarkerResolvedCalls : &hitMarkerFallbackCalls);
        }
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

void DrawHitLog()
{
    if (!keyValidated || !hitLogEnabled) return;
    const ULONGLONG now = GetTickCount64();
    AcquireSRWLockExclusive(&hitLogLock);

    const ULONGLONG lifetime = static_cast<ULONGLONG>(hitLogDuration * 1000.0f);
    std::vector<HitLogEntry> snapshot;
    hitLogEntries.erase(std::remove_if(hitLogEntries.begin(), hitLogEntries.end(),
        [now, lifetime](const HitLogEntry& entry) {
            return now < entry.createdAt || now - entry.createdAt >= lifetime;
        }), hitLogEntries.end());
    snapshot = hitLogEntries;
    ReleaseSRWLockExclusive(&hitLogLock);

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
    const float fontSize = 25.0f;
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    for (size_t i = 0; i < snapshot.size(); ++i) {
        const HitLogEntry& entry = snapshot[snapshot.size() - 1 - i];
        const float elapsed = static_cast<float>(now - entry.createdAt);
        const float fadeStart = static_cast<float>(lifetime) * 0.72f;
        const float alpha = elapsed <= fadeStart ? 1.0f :
            fmaxf(0.0f, 1.0f - (elapsed - fadeStart) / (static_cast<float>(lifetime) - fadeStart));
        char text[256];
        sprintf_s(text, "[Hitlog] -> %s | Damage: %d | Hitbox: %s",
            entry.enemyName, entry.damage, entry.hitbox);
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, 1000.0f, 0.0f, text);
        const ImVec2 pos((display.x - textSize.x) * 0.5f, 34.0f + static_cast<float>(i) * 31.0f);
        const int a = static_cast<int>(255.0f * alpha);
        const ImU32 shadow = IM_COL32(0, 0, 0, a);
        const ImU32 white = IM_COL32(255, 255, 255, a);
        draw->AddText(font, fontSize, ImVec2(pos.x - 2.0f, pos.y), shadow, text);
        draw->AddText(font, fontSize, ImVec2(pos.x + 2.0f, pos.y), shadow, text);
        draw->AddText(font, fontSize, ImVec2(pos.x, pos.y - 2.0f), shadow, text);
        draw->AddText(font, fontSize, ImVec2(pos.x, pos.y + 2.0f), shadow, text);
        draw->AddText(font, fontSize, pos, white, text);
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

static bool ReadDump1NativeTargetDamage(const HitCasterCer& castResult,
    void* player, int& damage)
{
    damage = 0;
    if (!castResult.hitDictionary || !player) return false;
    __try {
        const uintptr_t targetHitController = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(player) + 0xF0);
        const uintptr_t dictionary = castResult.hitDictionary;
        const uintptr_t entries = *reinterpret_cast<uintptr_t*>(dictionary + 0x18);
        const int count = *reinterpret_cast<int*>(dictionary + 0x20);
        if (!targetHitController || !entries || count <= 0 || count > 128)
            return false;
        // dump1 cer.cgta is Dictionary<bal,ccr>. Entry layout is hash/next +0x00,
        // bal key +0x08, ccr value +0x10, stride 0x18. ccr.cgid damage is +0x2C.
        for (int i = 0; i < count; ++i) {
            const uintptr_t entry = entries + 0x20 + static_cast<uintptr_t>(i) * 0x18;
            if (*reinterpret_cast<int*>(entry + 0x00) < 0) continue;
            if (*reinterpret_cast<uintptr_t*>(entry + 0x08) != targetHitController)
                continue;
            const uintptr_t hit = *reinterpret_cast<uintptr_t*>(entry + 0x10);
            if (!hit) return false;
            const int nativeDamage = *reinterpret_cast<int*>(hit + 0x2C);
            if (nativeDamage <= 0 || nativeDamage > 500) return false;
            damage = nativeDamage;
            return true;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { damage = 0; }
    return false;
}

static bool IsStrictlyPenetrableSurface(int type)
{
    // dump1 cex. Fail closed: only thin/breakable materials are accepted without
    // weapon-specific native damage data. Concrete, brick, metal, ground, etc. reject.
    return type == 1 || type == 2 || type == 3 || type == 4 ||
        type == 5 || type == 6 || type == 11 || type == 12 ||
        type == 22 || type == 26;
}

static bool ValidateAimbotRayPath(Vector3 origin, Vector3 direction, float distance,
    void* targetPlayer, bool allowWallbang)
{
    if (!targetPlayer || !o_Physics_RaycastAll || !o_RaycastHit_get_collider ||
        !o_Component_GetInParent || !g_PlayerControllerReflectionType)
        return false;
    __try {
        Il2CppArray* rayHits = o_Physics_RaycastAll(
            origin, direction, distance + 0.35f, -1, 1, nullptr);
        const uintptr_t array = reinterpret_cast<uintptr_t>(rayHits);
        const size_t count = array ? *reinterpret_cast<size_t*>(array + 0x18) : 0;
        if (!array || count == 0 || count > 128) return false;

        // RaycastAll is not guaranteed to be ordered. Find the target's nearest hit and
        // independently inspect every collider before it. No collider-free fallback.
        float targetDistance = 3.402823466e+38F;
        for (size_t i = 0; i < count; ++i) {
            RaycastHitNative* hit = reinterpret_cast<RaycastHitNative*>(
                array + 0x20 + i * sizeof(RaycastHitNative));
            if (!isfinite(hit->distance) || hit->distance < 0.0f) continue;
            const uintptr_t collider = o_RaycastHit_get_collider(hit, nullptr);
            if (!collider) continue;
            const uintptr_t hitPlayer = o_Component_GetInParent(
                collider, reinterpret_cast<uintptr_t>(g_PlayerControllerReflectionType),
                true, nullptr);
            if (hitPlayer == reinterpret_cast<uintptr_t>(targetPlayer) &&
                hit->distance < targetDistance)
                targetDistance = hit->distance;
        }
        if (!isfinite(targetDistance) || targetDistance == 3.402823466e+38F)
            return false;

        int obstructionCount = 0;
        for (size_t i = 0; i < count; ++i) {
            RaycastHitNative* hit = reinterpret_cast<RaycastHitNative*>(
                array + 0x20 + i * sizeof(RaycastHitNative));
            if (!isfinite(hit->distance) || hit->distance < 0.0f ||
                hit->distance >= targetDistance - 0.05f)
                continue;
            const uintptr_t collider = o_RaycastHit_get_collider(hit, nullptr);
            if (!collider) return false;
            const uintptr_t hitPlayer = o_Component_GetInParent(
                collider, reinterpret_cast<uintptr_t>(g_PlayerControllerReflectionType),
                true, nullptr);
            if (hitPlayer) {
                // Any different player before the selected target blocks the shot.
                if (hitPlayer != reinterpret_cast<uintptr_t>(targetPlayer)) return false;
                continue;
            }
            ++obstructionCount;
            if (!allowWallbang || obstructionCount > 1 || !o_Component_get_tag ||
                !o_SurfaceType_FromTag)
                return false;
            Il2CppString* tag = o_Component_get_tag(collider, nullptr);
            const int surface = tag ? o_SurfaceType_FromTag(tag, nullptr) : 0;
            if (!IsStrictlyPenetrableSurface(surface)) return false;
        }
        return obstructionCount == 0 || (allowWallbang && obstructionCount == 1);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool FindVisibleAimbotDirection(Vector3 origin, bool forceValidatedPath,
    const HitCasterCeq* castParameters, const Il2CppMethod* castMethod,
    Vector3& outDirection)
{
    void* localPlayer = GetLocalPC();
    if (!localPlayer) return false;
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
        if (GetPCHealth(player) == 0) continue;

        Vector3 head;
        if (!GetAimbotHead(player, head)) continue;
        const Vector3 delta(head.x - origin.x, head.y - origin.y, head.z - origin.z);
        const float distance = delta.Length();
        if (!isfinite(distance) || distance < 0.5f) continue;
        const Vector3 candidate(delta.x / distance, delta.y / distance, delta.z / distance);

        bool reachable = false;
        if (castParameters && castMethod && o_HitCaster_Cast) {
            // This is the old working algorithm ported to dump1's value ABI: let the
            // game calculate penetration and damage, then accept only the exact target.
            const HitCasterCer probe = o_HitCaster_Cast(
                origin, candidate, distance + 0.35f, *castParameters, castMethod);
            int nativeDamage = 0;
            const bool hitExactTarget =
                ReadDump1NativeTargetDamage(probe, player, nativeDamage);
            int penetrationCount = 0;
            __try {
                penetrationCount = probe.penetrations ?
                    *reinterpret_cast<int*>(probe.penetrations + 0x18) : 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { penetrationCount = -1; }
            const bool directShot = penetrationCount == 0;
            const bool validWallbang = aimbotAutoWall &&
                penetrationCount > 0 && penetrationCount <= 64 &&
                nativeDamage >= static_cast<int>(aimbotAutoWallMinDamage);
            reachable = hitExactTarget && (directShot || validWallbang);
        }
        else if (!forceValidatedPath && !aimbotVisibleCheck) {
            reachable = true;
        }
        else {
            // Before the first real ceq is observed, only a verified geometric path is
            // accepted. An Auto Wall bootstrap is later revalidated inside HitCaster.
            reachable = ValidateAimbotRayPath(origin, candidate, distance, player,
                aimbotAutoWall);
        }
        if (!reachable) continue;
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

HitCasterCer __fastcall hk_HitCaster_Cast(Vector3 origin, Vector3 direction,
    float maxDistance, HitCasterCeq hitParameters, const Il2CppMethod* method)
{
    bool applyVisibleSnapAfterCast = false;
    Vector3 visibleSnapDirection;
    if (insideLocalGunFire) {
        if (hitParameters.damageDefinition &&
            hitParameters.damageDefinition != aimbotLastHitParameters) {
            const uint32_t newHandle = g_il2cpp.gchandle_new ?
                g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(
                    hitParameters.damageDefinition), false) : 0;
            if (newHandle || !g_il2cpp.gchandle_new) {
                if (aimbotLastHitParametersHandle && g_il2cpp.gchandle_free)
                    g_il2cpp.gchandle_free(aimbotLastHitParametersHandle);
                aimbotLastHitParameters = hitParameters.damageDefinition;
                aimbotLastHitParametersHandle = newHandle;
            }
        }
        aimbotLastCeq = hitParameters;
        aimbotLastCeqValid = hitParameters.damageDefinition != 0 && method != nullptr;
        aimbotLastCeqGun = activeLocalWeaponController;
        aimbotLastHitCasterMethod = method;
    }
    if (keyValidated && insideLocalGunFire && (aimbotEnabled || visibleAimbotEnabled)) {
        InterlockedIncrement(&aimbotShots);
        Vector3 targetDirection;
        if (FindVisibleAimbotDirection(origin, true, &hitParameters, method, targetDirection)) {
            direction = targetDirection;
            visibleSnapDirection = targetDirection;
            applyVisibleSnapAfterCast = visibleAimbotEnabled;
            InterlockedIncrement(&aimbotApplied);
            strcpy_s(aimbotStatus, visibleAimbotEnabled ?
                "Target locked at dump1 HitCaster" :
                "Silent direction applied at dump1 HitCaster");
        } else {
            if (insideAutoFireRequest) {
                InterlockedIncrement(&aimbotAutoFireRejected);
                strcpy_s(aimbotStatus, "Auto Fire suppressed: native cast rejected path");
                HitCasterCer rejected = {};
                return rejected;
            }
            strcpy_s(aimbotStatus, "No reachable enemy; original shot kept");
        }
    }
    else if (keyValidated && insideLocalGunFire && noSpreadEnabled) {
        __try {
            const uintptr_t camera = GetCamera();
            const uintptr_t transform = camera && o_Component_get_transform ?
                o_Component_get_transform(camera) : 0;
            if (transform && o_Transform_get_forward)
                direction = o_Transform_get_forward(transform);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    const bool markLocalHitCast = keyValidated && hitLogEnabled && insideLocalGunFire;
    if (markLocalHitCast) ++activeLocalHitCastDepth;
    const HitCasterCer result = o_HitCaster_Cast(
        origin, direction, maxDistance, hitParameters, method);
    if (markLocalHitCast) --activeLocalHitCastDepth;

    const float castLength = result.start.Distance(result.end);
    const bool validResult = isfinite(result.start.x) && isfinite(result.start.y) &&
        isfinite(result.start.z) && isfinite(result.end.x) && isfinite(result.end.y) &&
        isfinite(result.end.z) && isfinite(castLength) && castLength >= 0.01f &&
        castLength <= 400.0f;
    if (insideAutoFireRequest && validResult)
        InterlockedIncrement(&aimbotAutoFired);
    if (validResult && applyVisibleSnapAfterCast) {
        BeginVisibleAimbotCameraSnap(visibleSnapDirection);
        strcpy_s(aimbotStatus, "Shot confirmed; visible camera snap applied");
    }

    if (keyValidated && insideLocalGunFire && validResult &&
        (hitMarkerEnabled || hitLogEnabled || bulletTracerEnabled || bulletImpactsEnabled)) {
        const ULONGLONG now = GetTickCount64();
        if (bulletImpactsEnabled) {
            __try {
                const uintptr_t list = result.penetrations;
                const int count = list ? *reinterpret_cast<int*>(list + 0x18) : 0;
                const uintptr_t items = list ? *reinterpret_cast<uintptr_t*>(list + 0x10) : 0;
                if (items && count > 0 && count <= 64) {
                    for (int i = 0; i < count; ++i) {
                        const uintptr_t item = items + 0x20 + static_cast<uintptr_t>(i) * 0x1C;
                        const Vector3 entry = *reinterpret_cast<Vector3*>(item + 0x00);
                        const Vector3 exit = *reinterpret_cast<Vector3*>(item + 0x0C);
                        if (IsPointOnBulletCast(entry, result.start, direction, maxDistance))
                            AddBulletImpact(entry, false);
                        if (IsPointOnBulletCast(exit, result.start, direction, maxDistance))
                            AddBulletImpact(exit, false);
                    }
                }
                AddBulletImpact(result.end, false);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
        if (hitMarkerEnabled || hitLogEnabled) {
            AcquireSRWLockExclusive(&hitMarkerLock);
            latestHitMarkerCastStart = result.start;
            latestHitMarkerCastEnd = result.end;
            latestHitMarkerCastAt = now;
            latestHitMarkerCastValid = true;
            ReleaseSRWLockExclusive(&hitMarkerLock);
        }
        if (bulletTracerEnabled) {
            AcquireSRWLockExclusive(&bulletTracerLock);
            bulletTracers.push_back({ result.start, result.end, now });
            if (bulletTracers.size() > 128)
                bulletTracers.erase(bulletTracers.begin(),
                    bulletTracers.begin() + (bulletTracers.size() - 128));
            ReleaseSRWLockExclusive(&bulletTracerLock);
        }
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
        const uintptr_t ownerPlayer = instance ? *(uintptr_t*)(instance + 0x58) : 0;
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

static ULONGLONG GetNativeAutoFireIntervalMs(uintptr_t gun)
{
    if (!gun || !base) return 100;
    __try {
        const uintptr_t parameters = *reinterpret_cast<uintptr_t*>(gun + 0x180);
        if (!parameters) return 100;
        using GetFireRateFn = int(__fastcall*)(uintptr_t, const Il2CppMethod*);
        const int fireRate = reinterpret_cast<GetFireRateFn>(base + 0xA24FF0)(parameters, nullptr);
        if (fireRate < 30 || fireRate > 3000) return 100;
        ULONGLONG interval = static_cast<ULONGLONG>(60000 / fireRate);
        if (interval < 20) interval = 20;
        if (interval > 1000) interval = 1000;
        return interval;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 100; }
}

static bool HasVisibleTargetBeforeNativeFire(uintptr_t gun)
{
    // Auto Fire always requires a verified PlayerController collider. Auto Wall may
    // additionally authorize exactly one known penetrable surface before that target.
    Vector3 origin;
    bool haveOrigin = false;
    __try {
        const uintptr_t camera = GetCamera();
        const uintptr_t transform = camera && o_Component_get_transform ?
            o_Component_get_transform(camera) : 0;
        if (transform && o_Transform_get_position) {
            origin = o_Transform_get_position(transform);
            haveOrigin = true;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { haveOrigin = false; }
    Vector3 direction;
    const bool haveCachedCast = aimbotLastCeqValid && aimbotLastCeqGun == gun &&
        aimbotLastHitCasterMethod != nullptr;
    const HitCasterCeq* cached = haveCachedCast ? &aimbotLastCeq : nullptr;
    const Il2CppMethod* cachedMethod = haveCachedCast ? aimbotLastHitCasterMethod : nullptr;
    return haveOrigin && FindVisibleAimbotDirection(
        origin, true, cached, cachedMethod, direction);
}

void __fastcall hk_GunController_Command(uintptr_t instance, uintptr_t command, float frameTime, float commandTime, const Il2CppMethod* method)
{
    bool injectNativeFire = false;
    bool originalPrimaryFire = false;
    const ULONGLONG now = GetTickCount64();
    __try {
        const uintptr_t currentWeapon = GetCurrentLocalWeaponController();
        const uintptr_t owner = instance ? *reinterpret_cast<uintptr_t*>(instance + 0x20) : 0;
        const bool isLocalGun = instance && command &&
            ((currentWeapon && instance == currentWeapon) ||
             (liveHudLocalPlayer && owner == liveHudLocalPlayer));
        const bool wantsAutoFire = keyValidated && isLocalGun && aimbotAutoFire &&
            (aimbotEnabled || visibleAimbotEnabled);
        if (wantsAutoFire && now >= aimbotAutoFireNextDecisionAt) {
            if (HasVisibleTargetBeforeNativeFire(instance)) {
                originalPrimaryFire = *reinterpret_cast<bool*>(command + 0x10);
                *reinterpret_cast<bool*>(command + 0x10) = true;
                injectNativeFire = true;
                // Persist across this command call: some weapon states schedule the
                // actual Fire/HitCaster after wfz returns.
                aimbotAutoFirePendingGun = instance;
                aimbotAutoFirePendingUntil = now + GetNativeAutoFireIntervalMs(instance) + 100;
                aimbotAutoFireNextDecisionAt = now + GetNativeAutoFireIntervalMs(instance);
                InterlockedIncrement(&aimbotAutoFireNativeCommands);
            } else {
                aimbotAutoFireNextDecisionAt = now;
                InterlockedIncrement(&aimbotAutoFireRejected);
                strcpy_s(aimbotStatus, "Auto Fire blocked: no direct shot or valid wallbang");
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { injectNativeFire = false; }

    o_GunController_Command(instance, command, frameTime, commandTime, method);

    if (injectNativeFire) {
        __try { *reinterpret_cast<bool*>(command + 0x10) = originalPrimaryFire; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    if (aimbotAutoFirePendingUntil && now > aimbotAutoFirePendingUntil) {
        aimbotAutoFirePendingGun = 0;
        aimbotAutoFirePendingUntil = 0;
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
    if (isLocalGun && (infinityAmmo || doubleTapEnabled)) {
        __try {
            ammoBeforeShot = *reinterpret_cast<short*>(instance + OFFSET_CURRENT_AMMO);
            if (infinityAmmo && ammoBeforeShot >= 0 && ammoBeforeShot < 1000) {
                frozenAmmoWeapon = instance;
                frozenAmmoValue = ammoBeforeShot;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { ammoBeforeShot = -1; }
    }
    const bool fireSecondShot = isLocalGun && doubleTapEnabled &&
        (infinityAmmo || ammoBeforeShot >= 2);
    const ULONGLONG fireNow = GetTickCount64();
    const bool persistentAutoRequest = isLocalGun && instance == aimbotAutoFirePendingGun &&
        fireNow <= aimbotAutoFirePendingUntil;
    insideAutoFireRequest = persistentAutoRequest;
    insideLocalGunFire = isLocalGun;
    uintptr_t doubleTapRecoilController = 0;
    unsigned char doubleTapRecoilState[0x28] = {};
    float doubleTapRecoilProgress = 0.0f;
    bool capturedDoubleTapRecoil = false;
    if (fireSecondShot) {
        __try {
            // GunController.cehb (ckf) owns the accumulated recoil/camera-deviation
            // state. Save it before the pair so the extra Fire cannot stack recoil.
            doubleTapRecoilController = *reinterpret_cast<uintptr_t*>(instance + 0xF8);
            if (doubleTapRecoilController) {
                memcpy(doubleTapRecoilState,
                    reinterpret_cast<void*>(doubleTapRecoilController + 0x10),
                    sizeof(doubleTapRecoilState));
                doubleTapRecoilProgress = *reinterpret_cast<float*>(doubleTapRecoilController + 0x50);
                capturedDoubleTapRecoil = true;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { capturedDoubleTapRecoil = false; }
    }
    if (isLocalGun && hitLogEnabled) ++activeLocalHitCastDepth;
    o_GunController_Fire(instance, playSound, method);
    if (isLocalGun && hitLogEnabled) --activeLocalHitCastDepth;
    if (fireSecondShot) {
        // Keep the real second cast/damage/ammo path, then restore only the native
        // recoil accumulator. Never lock or rewrite camera aim after the shot.
        if (isLocalGun && hitLogEnabled) ++activeLocalHitCastDepth;
        o_GunController_Fire(instance, playSound, method);
        if (isLocalGun && hitLogEnabled) --activeLocalHitCastDepth;
        if (capturedDoubleTapRecoil) {
            __try {
                memcpy(reinterpret_cast<void*>(doubleTapRecoilController + 0x10),
                    doubleTapRecoilState, sizeof(doubleTapRecoilState));
                *reinterpret_cast<float*>(doubleTapRecoilController + 0x50) = doubleTapRecoilProgress;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
        InterlockedIncrement(&doubleTapExtraShots);
    }
    insideLocalGunFire = false;
    insideAutoFireRequest = false;
    if (persistentAutoRequest) {
        aimbotAutoFirePendingGun = 0;
        aimbotAutoFirePendingUntil = 0;
    }
    if (isLocalGun && infinityAmmo && ammoBeforeShot >= 0) {
        __try {
            *reinterpret_cast<short*>(instance + OFFSET_CURRENT_AMMO) = ammoBeforeShot;
            using SetAmmoFn = void(__fastcall*)(uintptr_t, short, const Il2CppMethod*);
            reinterpret_cast<SetAmmoFn>(base + 0xA1F060)(instance, ammoBeforeShot, nullptr);
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
    // Auto Fire is injected in dump1 GunController.vfl (native weapon command processing).
    // Never call private GunController.Fire from the movement loop.
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
            reinterpret_cast<SetAmmoFn>(base + 0xA1F060)(weaponController, frozenAmmoValue, nullptr);
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

    const LONG64 releaseGraceUntil = InterlockedCompareExchange64(
        &pixelSurfReleaseGraceUntil, 0, 0);
    if (!jbActive && pixelSurfReleaseHop &&
        releaseGraceUntil > static_cast<LONG64>(GetTickCount64()))
        return true;

    if (jbActive) return false;

    if (grounded && instance && instance == pixelSurfCharacterController) {
        InterlockedExchange64(&pixelSurfReleasedAt, 0);
        pixelSurfCharacterController = 0;
    }
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
        for (WeaponChamsRenderer& entry : weaponChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
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

static void ConfigureOutlineChamsMaterial(uintptr_t material)
{
    if (!material) return;
    if (o_Material_SetFloat && g_il2cpp.string_new) {
        // Inverted hull: clone is enlarged and only its back faces are rendered.
        o_Material_SetFloat(material, g_il2cpp.string_new("_Cull"), 1.0f);
        o_Material_SetFloat(material, g_il2cpp.string_new("_ZWrite"), 0.0f);
        o_Material_SetFloat(material, g_il2cpp.string_new("_ZTest"), 4.0f);
        o_Material_SetFloat(material, g_il2cpp.string_new("_SrcBlend"), 5.0f);
        o_Material_SetFloat(material, g_il2cpp.string_new("_DstBlend"), 10.0f);
    }
    if (o_Material_set_renderQueue) o_Material_set_renderQueue(material, 3001);
}

static bool CreateOutlineCloneUnsafe(uintptr_t renderer, uintptr_t material,
    uintptr_t* outlineObject, uintptr_t* outlineRenderer)
{
    if (!renderer || !material || !outlineObject || !outlineRenderer || !base ||
        !o_Component_get_gameObject || !o_Component_get_transform || !o_Renderer_set_material) return false;
    using InstantiateFn = uintptr_t(__fastcall*)(uintptr_t, const Il2CppMethod*);
    using DestroyFn = void(__fastcall*)(uintptr_t, const Il2CppMethod*);
    using GetScaleFn = Vector3(__fastcall*)(uintptr_t, const Il2CppMethod*);
    using SetScaleFn = void(__fastcall*)(uintptr_t, Vector3, const Il2CppMethod*);
    __try {
        const uintptr_t sourceObject = o_Component_get_gameObject(renderer);
        if (!sourceObject) return false;
        // Instantiate(Component) preserves SkinnedMeshRenderer mesh and bone bindings.
        const uintptr_t clonedComponent = reinterpret_cast<InstantiateFn>(base + OFFSET_OBJECT_INSTANTIATE)(renderer, nullptr);
        if (!clonedComponent) return false;
        const uintptr_t object = o_Component_get_gameObject(clonedComponent);
        const uintptr_t transform = o_Component_get_transform(clonedComponent);
        if (!object || !transform) { reinterpret_cast<DestroyFn>(base + OFFSET_OBJECT_DESTROY)(object, nullptr); return false; }
        Vector3 scale = reinterpret_cast<GetScaleFn>(base + OFFSET_TRANSFORM_GET_LOCALSCALE)(transform, nullptr);
        scale.x *= 1.035f; scale.y *= 1.035f; scale.z *= 1.035f;
        reinterpret_cast<SetScaleFn>(base + OFFSET_TRANSFORM_SET_LOCALSCALE)(transform, scale, nullptr);
        o_Renderer_set_material(clonedComponent, material);
        *outlineObject = object;
        *outlineRenderer = clonedComponent;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void DestroyOutlineCloneUnsafe(uintptr_t* outlineObject, uintptr_t* outlineRenderer)
{
    if (!outlineObject || !outlineRenderer) return;
    __try {
        if (*outlineObject && base)
            reinterpret_cast<void(__fastcall*)(uintptr_t, const Il2CppMethod*)>(base + OFFSET_OBJECT_DESTROY)(*outlineObject, nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    *outlineObject = 0;
    *outlineRenderer = 0;
}

static bool ReadManagedArrayCountUnsafe(uintptr_t arrayAddress, size_t* count)
{
    if (!arrayAddress || !count) return false;
    __try {
        *count = *reinterpret_cast<size_t*>(arrayAddress + 0x18);
        return *count > 0 && *count <= 64;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { *count = 0; return false; }
}

static bool ReadManagedArrayElementUnsafe(uintptr_t arrayAddress, size_t count,
    size_t index, uintptr_t* value)
{
    if (!arrayAddress || !value || index >= count) return false;
    __try {
        *value = *reinterpret_cast<uintptr_t*>(arrayAddress + 0x20 + index * sizeof(uintptr_t));
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { *value = 0; return false; }
}

static bool ProbeTaggedPenetrableSurfaceUnsafe(Vector3 origin, Vector3 direction, float maxDistance,
    uintptr_t rendererType, uintptr_t* renderer, int* surfaceType,
    bool* physicalHit, bool* typedHit)
{
    if (!renderer || !surfaceType || !physicalHit || !typedHit) return false;
    *renderer = 0; *surfaceType = 0; *physicalHit = false; *typedHit = false;
    if (!o_Physics_RaycastHit || !o_RaycastHit_get_collider || !o_Component_get_tag ||
        !o_SurfaceType_FromTag || !o_Component_GetInParent || !o_Component_GetInChildren ||
        !rendererType) return false;
    __try {
        RaycastHitNative hit = {};
        if (!o_Physics_RaycastHit(origin, direction, &hit, maxDistance, -1, 1, nullptr)) return false;
        *physicalHit = true;
        const uintptr_t collider = o_RaycastHit_get_collider(&hit, nullptr);
        if (!collider) return false;
        // HitCaster resolves cjw from the map's serialized "SurfaceType/..." tag.
        // This path works without an enemy, a shot, or weapon-specific cjq state.
        Il2CppString* tag = o_Component_get_tag(collider, nullptr);
        const int type = tag ? o_SurfaceType_FromTag(tag, nullptr) : 0;
        const bool penetrableMapMaterial = (type >= 1 && type <= 9) ||
            type == 11 || type == 12 || type == 16 || type == 17 || type == 22 || type == 26;
        if (!penetrableMapMaterial) return false;
        *typedHit = true;
        *surfaceType = type;
        uintptr_t found = o_Component_GetInParent(collider, rendererType, true, nullptr);
        if (!found) found = o_Component_GetInChildren(collider, rendererType, true, nullptr);
        uintptr_t hierarchy = o_Component_get_transform ? o_Component_get_transform(collider) : 0;
        for (int depth = 0; !found && hierarchy && depth < 8; ++depth) {
            found = o_Component_GetInChildren(hierarchy, rendererType, true, nullptr);
            hierarchy = o_Transform_get_parent ? o_Transform_get_parent(hierarchy, nullptr) : 0;
        }
        if (!found) return false;
        *renderer = found;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        *renderer = 0; *surfaceType = 0; *physicalHit = false; *typedHit = false;
        return false;
    }
}

static const char* GetPenetrableSurfaceTypeName(int type)
{
    static const char* names[] = {
        "Unknown", "Glass", "Cardboard", "Metal Grate", "Wood", "Plaster", "Tile",
        "Metal", "Concrete", "Brick", "Solid Metal", "Thin Metal", "Thin Wood", "Character",
        "Ground", "Surface Trigger", "Snow Concrete", "Snow Metal", "Snow Ground",
        "Solid Metal No Hole", "Gravel", "Grass", "Paper", "Water", "Glass Waterfall",
        "Solid Wood", "Glass No Decal"
    };
    return type >= 0 && type < static_cast<int>(IM_ARRAYSIZE(names)) ? names[type] : "Penetrable";
}

static bool IsUnityObjectAliveUnsafe(uintptr_t object)
{
    if (!object || !o_Object_IsAlive) return false;
    __try { return o_Object_IsAlive(object, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void RestorePenetrableSurfaceEntry(PenetrableSurfaceVisual& entry)
{
    const bool rendererAlive = IsUnityObjectAliveUnsafe(entry.renderer);
    if (rendererAlive && !entry.originalMaterials.empty())
        SetRendererMaterialArray(entry.renderer, entry.originalMaterials);
    // Destroy only a live clone. Calling Destroy on a wrapper whose native side was
    // removed during death/scene teardown was the likely post-death crash path.
    if (IsUnityObjectAliveUnsafe(entry.outlineObject))
        DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
    else {
        entry.outlineObject = 0;
        entry.outlineRenderer = 0;
    }
    if (entry.rendererHandle && g_il2cpp.gchandle_free)
        g_il2cpp.gchandle_free(entry.rendererHandle);
    entry.rendererHandle = 0;
    entry.renderer = 0;
    entry.originalMaterials.clear();
}

static void RestorePenetrableSurfaceVisualization()
{
    for (PenetrableSurfaceVisual& entry : penetrableSurfaceVisuals)
        RestorePenetrableSurfaceEntry(entry);
    penetrableSurfaceVisuals.clear();
    penetrableSurfaceScanCursor = 0;
}

static bool EnsurePenetrableSurfaceMaterials()
{
    if (!penetrableSurfaceMaterial) {
        penetrableSurfaceMaterial = CreateWeaponChamsMaterial("Unlit/Transparent", true);
        if (!penetrableSurfaceMaterial)
            penetrableSurfaceMaterial = CreateWeaponChamsMaterial("Legacy Shaders/Transparent/Diffuse", true);
        if (penetrableSurfaceMaterial && g_il2cpp.gchandle_new)
            penetrableSurfaceMaterialHandle = g_il2cpp.gchandle_new(
                reinterpret_cast<Il2CppObject*>(penetrableSurfaceMaterial), false);
    }
    if (!penetrableSurfaceOutlineMaterial) {
        penetrableSurfaceOutlineMaterial = CreateWeaponChamsMaterial("Hidden/Internal-Colored", true);
        if (!penetrableSurfaceOutlineMaterial)
            penetrableSurfaceOutlineMaterial = CreateWeaponChamsMaterial("Unlit/Color", true);
        if (penetrableSurfaceOutlineMaterial) {
            ConfigureOutlineChamsMaterial(penetrableSurfaceOutlineMaterial);
            if (g_il2cpp.gchandle_new)
                penetrableSurfaceOutlineMaterialHandle = g_il2cpp.gchandle_new(
                    reinterpret_cast<Il2CppObject*>(penetrableSurfaceOutlineMaterial), false);
        }
    }
    if (!penetrableSurfaceMaterial || !penetrableSurfaceOutlineMaterial || !o_Material_set_color) return false;
    if (o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(penetrableSurfaceMaterial, g_il2cpp.string_new("_SrcBlend"), 5.0f);
        o_Material_SetFloat(penetrableSurfaceMaterial, g_il2cpp.string_new("_DstBlend"), 10.0f);
        o_Material_SetFloat(penetrableSurfaceMaterial, g_il2cpp.string_new("_ZWrite"), 0.0f);
        o_Material_SetFloat(penetrableSurfaceMaterial, g_il2cpp.string_new("_Cull"), 0.0f);
    }
    if (o_Material_set_renderQueue) o_Material_set_renderQueue(penetrableSurfaceMaterial, 3000);
    o_Material_set_color(penetrableSurfaceMaterial, Color(
        penetrableSurfaceFillColor[0], penetrableSurfaceFillColor[1],
        penetrableSurfaceFillColor[2], penetrableSurfaceAlpha));
    o_Material_set_color(penetrableSurfaceOutlineMaterial, Color(
        penetrableSurfaceOutlineColor[0], penetrableSurfaceOutlineColor[1],
        penetrableSurfaceOutlineColor[2], 1.0f));
    return true;
}

static bool CapturePenetrableSurfaceRenderer(uintptr_t renderer, int surfaceType, ULONGLONG now)
{
    if (!renderer || !IsUnityObjectAliveUnsafe(renderer) || !o_Renderer_get_materials) return false;
    for (PenetrableSurfaceVisual& entry : penetrableSurfaceVisuals) {
        if (entry.renderer != renderer) continue;
        entry.surfaceType = surfaceType;
        entry.lastSeen = now;
        return true;
    }
    if (penetrableSurfaceVisuals.size() >= 48) return false;
    Il2CppArray* materials = o_Renderer_get_materials(renderer);
    const uintptr_t arrayAddress = reinterpret_cast<uintptr_t>(materials);
    size_t count = 0;
    if (!ReadManagedArrayCountUnsafe(arrayAddress, &count)) return false;
    PenetrableSurfaceVisual entry{};
    entry.renderer = renderer;
    entry.surfaceType = surfaceType;
    entry.lastSeen = now;
    entry.rendererHandle = g_il2cpp.gchandle_new ?
        g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(renderer), false) : 0;
    entry.originalMaterials.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        uintptr_t material = 0;
        if (!ReadManagedArrayElementUnsafe(arrayAddress, count, i, &material)) {
            if (entry.rendererHandle && g_il2cpp.gchandle_free) g_il2cpp.gchandle_free(entry.rendererHandle);
            return false;
        }
        entry.originalMaterials.push_back(material);
    }
    penetrableSurfaceVisuals.push_back(std::move(entry));
    return true;
}

static Vector3 GetPenetrableWorldScanDirection(size_t index)
{
    // 512 directions is dense enough to strike ordinary wall panels at 100-250 m.
    constexpr float count = 512.0f;
    constexpr float goldenAngle = 2.39996322972865332f;
    const float i = static_cast<float>(index % 512);
    const float y = 1.0f - 2.0f * ((i + 0.5f) / count);
    const float radius = sqrtf((std::max)(0.0f, 1.0f - y * y));
    const float theta = goldenAngle * i;
    return Vector3(cosf(theta) * radius, y, sinf(theta) * radius);
}

static void ApplyPenetrableSurfaceEntry(PenetrableSurfaceVisual& entry)
{
    if (!IsUnityObjectAliveUnsafe(entry.renderer)) return;
    std::vector<uintptr_t> transparentMaterials(
        entry.originalMaterials.size(), penetrableSurfaceMaterial);
    SetRendererMaterialArray(entry.renderer, transparentMaterials);
    // Never instantiate/scale map renderers: static-batched walls produced phantom
    // invisible objects. Highlight only the exact confirmed wall Renderer in place.
}

static void UpdatePenetrableSurfaceVisualization()
{
    if (!penetrableSurfacesEnabled || !keyValidated) {
        if (!penetrableSurfaceVisuals.empty()) RestorePenetrableSurfaceVisualization();
        strcpy_s(penetrableSurfaceStatus, "Disabled");
        return;
    }
    void* localPlayer = GetLocalPC();
    const unsigned char localTeam = GetPCTeam(localPlayer);
    const int localHealth = GetPCHealth(localPlayer);
    if (!liveHudLocalPlayer || !localPlayer || localTeam == 0 || localTeam == 3 || localHealth == 0) {
        // HUD can keep updating during death/spectator transitions. Drop every cached
        // Unity object before scene/player teardown can invalidate its native pointer.
        if (!penetrableSurfaceVisuals.empty()) RestorePenetrableSurfaceVisualization();
        strcpy_s(penetrableSurfaceStatus, "Paused while dead/spectating");
        return;
    }
    const ULONGLONG now = GetTickCount64();
    if (now < penetrableSurfaceNextScan) return;
    penetrableSurfaceNextScan = now + 50;
    if (!EnsurePenetrableSurfaceMaterials()) {
        strcpy_s(penetrableSurfaceStatus, "Transparent/outline shader unavailable");
        return;
    }
    const uintptr_t camera = GetCamera();
    const uintptr_t transform = camera && o_Component_get_transform ? o_Component_get_transform(camera) : 0;
    if (!transform || !o_Transform_get_position) {
        if (!penetrableSurfaceVisuals.empty()) RestorePenetrableSurfaceVisualization();
        strcpy_s(penetrableSurfaceStatus, "Waiting for live camera");
        return;
    }
    Il2CppClass* rendererClass = g_il2cpp.find_class("UnityEngine", "Renderer");
    const Il2CppType* rendererIl2CppType = rendererClass && g_il2cpp.class_get_type ?
        g_il2cpp.class_get_type(rendererClass) : nullptr;
    Il2CppObject* reflectionType = rendererIl2CppType && g_il2cpp.type_get_object ?
        g_il2cpp.type_get_object(rendererIl2CppType) : nullptr;
    if (!reflectionType) {
        strcpy_s(penetrableSurfaceStatus, "Renderer type unavailable");
        return;
    }
    const Vector3 origin = o_Transform_get_position(transform);
    size_t physicalHits = 0;
    size_t typedHits = 0;
    size_t rendererHits = 0;
    // 32 rays every 50 ms: one complete dense 360-degree sweep in ~0.8 seconds.
    for (size_t ray = 0; ray < 32; ++ray) {
        const Vector3 direction = GetPenetrableWorldScanDirection(penetrableSurfaceScanCursor++);
        uintptr_t renderer = 0;
        int surfaceType = 0;
        bool physicalHit = false;
        bool typedHit = false;
        const bool resolved = ProbeTaggedPenetrableSurfaceUnsafe(origin, direction,
            penetrableSurfaceScanDistance, reinterpret_cast<uintptr_t>(reflectionType),
            &renderer, &surfaceType, &physicalHit, &typedHit);
        if (physicalHit) ++physicalHits;
        if (typedHit) ++typedHits;
        if (resolved && CapturePenetrableSurfaceRenderer(renderer, surfaceType, now))
            ++rendererHits;
    }
    // Remove stale/destroyed objects safely. A tagged map hit refreshes lastSeen.
    for (auto it = penetrableSurfaceVisuals.begin(); it != penetrableSurfaceVisuals.end();) {
        if (!IsUnityObjectAliveUnsafe(it->renderer) || now < it->lastSeen || now - it->lastSeen > 2500) {
            RestorePenetrableSurfaceEntry(*it);
            it = penetrableSurfaceVisuals.erase(it);
        } else {
            ApplyPenetrableSurfaceEntry(*it);
            ++it;
        }
    }
    sprintf_s(penetrableSurfaceStatus,
        "World scan: %zu wall(s) | physics %zu | typed %zu | renderer %zu",
        penetrableSurfaceVisuals.size(), physicalHits, typedHits, rendererHits);
}

static uintptr_t EnsureSelectedWeaponChamsMaterial()
{
    const int mode = (weaponChamsMode >= 0 && weaponChamsMode < 8) ? weaponChamsMode : 0;
    if (weaponChamsMaterials[mode]) return weaponChamsMaterials[mode];

    static const char* shaderCandidates[8][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr, nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        weaponChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (weaponChamsMaterials[mode]) break;
    }
    const uintptr_t material = weaponChamsMaterials[mode];
    if (!material) {
        static const char* names[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
        sprintf_s(weaponChamsStatus, "%s shader not found in this build", names[mode]);
        return 0;
    }
    if (g_il2cpp.gchandle_new)
        weaponChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    if (mode == 7) ConfigureOutlineChamsMaterial(material);
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
    const int mode = (armChamsMode >= 0 && armChamsMode < 8) ? armChamsMode : 0;
    if (armChamsMaterials[mode]) return armChamsMaterials[mode];
    static const char* shaderCandidates[8][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr, nullptr }
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
    if (mode == 7) ConfigureOutlineChamsMaterial(material);
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
        for (ArmChamsRenderer& entry : armChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
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
            if (originalMaterial) armChamsRenderers.push_back({ renderer, originalMaterial, 0, 0 });
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
    for (ArmChamsRenderer& entry : armChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            if (armChamsMode == 7) {
                if (o_Renderer_get_material && o_Renderer_get_material(entry.renderer) != entry.originalMaterial)
                    o_Renderer_set_material(entry.renderer, entry.originalMaterial);
                if (!entry.outlineObject)
                    CreateOutlineCloneUnsafe(entry.renderer, replacement, &entry.outlineObject, &entry.outlineRenderer);
            }
            else {
                DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
                const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
                if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(armChamsStatus, "Active");
}

static uintptr_t EnsureSelectedGloveChamsMaterial()
{
    const int mode = (gloveChamsMode >= 0 && gloveChamsMode < 8) ? gloveChamsMode : 0;
    if (gloveChamsMaterials[mode]) return gloveChamsMaterials[mode];
    static const char* shaderCandidates[8][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr, nullptr }
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
    if (mode == 7) ConfigureOutlineChamsMaterial(material);
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
        for (GloveChamsRenderer& entry : gloveChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
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
            if (originalMaterial) gloveChamsRenderers.push_back({ renderer, originalMaterial, 0, 0 });
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
    for (GloveChamsRenderer& entry : gloveChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            if (gloveChamsMode == 7) {
                if (o_Renderer_get_material && o_Renderer_get_material(entry.renderer) != entry.originalMaterial)
                    o_Renderer_set_material(entry.renderer, entry.originalMaterial);
                if (!entry.outlineObject)
                    CreateOutlineCloneUnsafe(entry.renderer, replacement, &entry.outlineObject, &entry.outlineRenderer);
            }
            else {
                DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
                const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
                if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    strcpy_s(gloveChamsStatus, "Active");
}

static uintptr_t GetCurrentLocalCharacterLodGroup()
{
    __try {
        const uintptr_t lodGroup = liveHudLocalPlayer ? *reinterpret_cast<uintptr_t*>(liveHudLocalPlayer + 0x130) : 0;
        if (lodGroup) currentLocalCharacterLodGroupCache = lodGroup;
        return lodGroup ? lodGroup : currentLocalCharacterLodGroupCache;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return currentLocalCharacterLodGroupCache; }
}

static uintptr_t EnsureSelectedLocalPlayerChamsMaterial()
{
    const int mode = (localPlayerChamsMode >= 0 && localPlayerChamsMode < 8) ? localPlayerChamsMode : 0;
    if (localPlayerChamsMaterials[mode]) return localPlayerChamsMaterials[mode];
    static const char* shaderCandidates[8][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr, nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        localPlayerChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (localPlayerChamsMaterials[mode]) break;
    }
    const uintptr_t material = localPlayerChamsMaterials[mode];
    if (!material) { strcpy_s(localPlayerChamsStatus, "Selected local shader not found"); return 0; }
    if (g_il2cpp.gchandle_new)
        localPlayerChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    if (mode == 7) ConfigureOutlineChamsMaterial(material);
    return material;
}

static Color GetLocalPlayerChamsDisplayColor()
{
    if (localPlayerChamsMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * localPlayerChamsAnimationSpeed;
        return Color(0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.0943951f),
            0.5f + 0.5f * sinf(t + 4.1887902f), 1.0f);
    }
    if (localPlayerChamsMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * localPlayerChamsAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(localPlayerChamsColor[0] * intensity, localPlayerChamsColor[1] * intensity,
            localPlayerChamsColor[2] * intensity, 1.0f);
    }
    return Color(localPlayerChamsColor[0], localPlayerChamsColor[1], localPlayerChamsColor[2],
        (localPlayerChamsMode == 1 || localPlayerChamsMode == 4) ? localPlayerChamsAlpha : 1.0f);
}

static void RestoreLocalPlayerChams()
{
    if (o_Renderer_set_material) {
        for (LocalPlayerChamsRenderer& entry : localPlayerChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
            __try {
                o_Renderer_set_material(entry.renderer, entry.originalMaterial);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    localPlayerChamsRenderers.clear();
}

static void CaptureLocalPlayerChamsRenderer(uintptr_t characterLodGroup)
{
    RestoreLocalPlayerChams();
    if (!characterLodGroup || !o_Renderer_get_material) {
        strcpy_s(localPlayerChamsStatus, "Local player renderer not found");
        return;
    }
    __try {
        const uintptr_t renderer = *reinterpret_cast<uintptr_t*>(characterLodGroup + 0x60);
        if (renderer) {
            const uintptr_t originalMaterial = o_Renderer_get_material(renderer);
            if (originalMaterial) localPlayerChamsRenderers.push_back({ renderer, originalMaterial, 0, 0 });
        }
        localPlayerChamsLodGroup = characterLodGroup;
        sprintf_s(localPlayerChamsStatus, "Captured %zu local renderer(s)", localPlayerChamsRenderers.size());
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        localPlayerChamsRenderers.clear();
        strcpy_s(localPlayerChamsStatus, "Local player renderer capture failed");
    }
}

static void UpdateLocalPlayerChams(uintptr_t knownCharacterLodGroup)
{
    if (!keyValidated || !o_Renderer_set_material || !o_Material_set_color) return;
    uintptr_t characterLodGroup = knownCharacterLodGroup ? knownCharacterLodGroup : localPlayerChamsLodGroup;
    if (!characterLodGroup) characterLodGroup = GetCurrentLocalCharacterLodGroup();
    if (!localPlayerChamsEnabled) {
        if (!localPlayerChamsRenderers.empty()) RestoreLocalPlayerChams();
        strcpy_s(localPlayerChamsStatus, "Disabled");
        return;
    }
    if (!characterLodGroup) { strcpy_s(localPlayerChamsStatus, "Waiting for local model"); return; }
    const uintptr_t replacement = EnsureSelectedLocalPlayerChamsMaterial();
    if (!replacement) return;
    bool rendererChanged = characterLodGroup != localPlayerChamsLodGroup || localPlayerChamsRenderers.empty();
    __try {
        const uintptr_t liveRenderer = *reinterpret_cast<uintptr_t*>(characterLodGroup + 0x60);
        if (!liveRenderer || localPlayerChamsRenderers.empty() || localPlayerChamsRenderers.front().renderer != liveRenderer)
            rendererChanged = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { rendererChanged = true; }
    if (rendererChanged) CaptureLocalPlayerChamsRenderer(characterLodGroup);
    if (localPlayerChamsRenderers.empty()) { strcpy_s(localPlayerChamsStatus, "Waiting for local renderer"); return; }
    if (localPlayerChamsMode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), localPlayerChamsMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), localPlayerChamsSmoothness);
    }
    if (o_Material_set_renderQueue)
        o_Material_set_renderQueue(replacement, (localPlayerChamsMode == 1 || localPlayerChamsMode == 4) ? 3000 : 2000);
    o_Material_set_color(replacement, GetLocalPlayerChamsDisplayColor());
    size_t applied = 0;
    for (LocalPlayerChamsRenderer& entry : localPlayerChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            if (localPlayerChamsMode == 7) {
                if (o_Renderer_get_material && o_Renderer_get_material(entry.renderer) != entry.originalMaterial)
                    o_Renderer_set_material(entry.renderer, entry.originalMaterial);
                if (!entry.outlineObject)
                    CreateOutlineCloneUnsafe(entry.renderer, replacement, &entry.outlineObject, &entry.outlineRenderer);
            }
            else {
                DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
                const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
                if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
            }
            ++applied;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    sprintf_s(localPlayerChamsStatus, "Active on %zu local renderer(s)", applied);
}

static void AnimateLocalPlayerChamsColor()
{
    if (!localPlayerChamsEnabled || (localPlayerChamsMode != 5 && localPlayerChamsMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - localPlayerChamsLastAnimationTick < 33) return;
    localPlayerChamsLastAnimationTick = now;
    const uintptr_t material = localPlayerChamsMaterials[localPlayerChamsMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetLocalPlayerChamsDisplayColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uintptr_t EnsureEnemyChamsMaterial(int requestedMode)
{
    const int mode = (requestedMode >= 0 && requestedMode < 9) ? requestedMode : 0;
    if (enemyChamsMaterials[mode]) return enemyChamsMaterials[mode];
    static const char* shaderCandidates[9][5] = {
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Transparent", "Legacy Shaders/Transparent/Diffuse", "Sprites/Default", "UI/Default", nullptr },
        { "Legacy Shaders/Diffuse", "Standard", "Legacy Shaders/VertexLit", "Diffuse", nullptr },
        { "Standard", "Legacy Shaders/Specular", "Legacy Shaders/Reflective/Diffuse", "Legacy Shaders/Diffuse", nullptr },
        { "Legacy Shaders/Transparent/Diffuse", "Legacy Shaders/Transparent/VertexLit", "Legacy Shaders/Transparent/Specular", "Unlit/Transparent", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", "UI/Default", nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", nullptr, nullptr },
        { "Hidden/Internal-Colored", "Unlit/Color", "Legacy Shaders/Unlit/Color", "Sprites/Default", nullptr }
    };
    const bool transparent = mode == 1 || mode == 4;
    for (const char* shaderName : shaderCandidates[mode]) {
        if (!shaderName) break;
        enemyChamsMaterials[mode] = CreateWeaponChamsMaterial(shaderName, transparent);
        if (enemyChamsMaterials[mode]) break;
    }
    if (mode == 8 && enemyChamsMaterials[mode] && o_Material_SetFloat && g_il2cpp.string_new) {
        // Draw the opaque x-ray silhouette after world geometry. The selected visual
        // material is queued immediately after it and overwrites this pass only on pixels
        // where its normal depth test succeeds.
        o_Material_SetFloat(enemyChamsMaterials[mode], g_il2cpp.string_new("_ZTest"), 8.0f);
        o_Material_SetFloat(enemyChamsMaterials[mode], g_il2cpp.string_new("_ZWrite"), 0.0f);
        o_Material_SetFloat(enemyChamsMaterials[mode], g_il2cpp.string_new("_Cull"), 0.0f);
        o_Material_SetFloat(enemyChamsMaterials[mode], g_il2cpp.string_new("_SrcBlend"), 1.0f);
        o_Material_SetFloat(enemyChamsMaterials[mode], g_il2cpp.string_new("_DstBlend"), 0.0f);
        if (o_Material_set_renderQueue) o_Material_set_renderQueue(enemyChamsMaterials[mode], 3998);
    }
    const uintptr_t material = enemyChamsMaterials[mode];
    if (mode == 7 && material) ConfigureOutlineChamsMaterial(material);
    if (!material) { strcpy_s(enemyChamsStatus, "Selected enemy shader not found"); return 0; }
    if (g_il2cpp.gchandle_new)
        enemyChamsMaterialHandles[mode] = g_il2cpp.gchandle_new(reinterpret_cast<Il2CppObject*>(material), false);
    return material;
}

static uintptr_t EnsureSelectedEnemyChamsMaterial()
{
    return EnsureEnemyChamsMaterial(enemyChamsMode);
}

static Color GetEnemyChamsDisplayColor()
{
    if (enemyChamsMode == 5) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * enemyChamsAnimationSpeed;
        return Color(0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.0943951f), 0.5f + 0.5f * sinf(t + 4.1887902f), 1.0f);
    }
    if (enemyChamsMode == 6) {
        const float t = static_cast<float>(GetTickCount64() % 60000) * 0.001f * enemyChamsAnimationSpeed;
        const float intensity = 0.2f + 0.8f * (0.5f + 0.5f * sinf(t));
        return Color(enemyChamsColor[0] * intensity, enemyChamsColor[1] * intensity, enemyChamsColor[2] * intensity, 1.0f);
    }
    return Color(enemyChamsColor[0], enemyChamsColor[1], enemyChamsColor[2],
        (enemyChamsMode == 1 || enemyChamsMode == 4) ? enemyChamsAlpha : 1.0f);
}

static void RestoreEnemyChams()
{
    if (o_Renderer_set_material) {
        for (EnemyChamsRenderer& entry : enemyChamsRenderers) {
            if (!entry.renderer || !entry.originalMaterial) continue;
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
            __try {
                o_Renderer_set_material(entry.renderer, entry.originalMaterial);
                if (o_Renderer_set_enabled) o_Renderer_set_enabled(entry.renderer, entry.originalEnabled);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    enemyChamsRenderers.clear();
}

static void CollectEnemyBodyRenderers(std::vector<uintptr_t>& renderers)
{
    renderers.clear();
    void* localPlayer = GetLocalPC();
    if (!localPlayer) return;
    const unsigned char localTeam = GetPCTeam(localPlayer);
    if (localTeam == 0 || localTeam == 3) return;

    void* players[64] = {};
    int playerCount = 0;
    CollectPlayers(players, 64, playerCount);
    for (int i = 0; i < playerCount; ++i) {
        void* player = players[i];
        if (!player || player == localPlayer) continue;
        const unsigned char team = GetPCTeam(player);
        if (team == 0 || team == 3 || team == localTeam) continue;
        __try {
            if (enemyChamsThroughWalls && o_ObjectOccludee_SetVisibleState) {
                const uintptr_t occlusionController = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(player) + 0xF8);
                const bool initialized = occlusionController && std::find(
                    enemyChamsInitializedOcclusionControllers.begin(),
                    enemyChamsInitializedOcclusionControllers.end(),
                    occlusionController) != enemyChamsInitializedOcclusionControllers.end();
                if (occlusionController && !initialized) {
                    // If Through Walls was enabled after this enemy had already become hidden,
                    // no future false transition was guaranteed. Wake all bpj listeners once;
                    // subsequent visibility changes are handled by the occlusion hook.
                    o_ObjectOccludee_SetVisibleState(occlusionController, false, nullptr);
                    o_ObjectOccludee_SetVisibleState(occlusionController, true, nullptr);
                    enemyChamsInitializedOcclusionControllers.push_back(occlusionController);
                }
            }
            // PlayerController::CharacterLodGroup +0x130 -> CharacterLodGroup::_meshRenderer +0x60.
            const uintptr_t characterLodGroup = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(player) + 0x130);
            const uintptr_t renderer = characterLodGroup ? *reinterpret_cast<uintptr_t*>(characterLodGroup + 0x60) : 0;
            if (renderer && std::find(renderers.begin(), renderers.end(), renderer) == renderers.end())
                renderers.push_back(renderer);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}

static void CaptureEnemyChamsRenderers(const std::vector<uintptr_t>& renderers)
{
    RestoreEnemyChams();
    if (!o_Renderer_get_material) { strcpy_s(enemyChamsStatus, "Enemy renderer API unavailable"); return; }
    for (uintptr_t renderer : renderers) {
        if (!renderer) continue;
        __try {
            const uintptr_t originalMaterial = o_Renderer_get_material(renderer);
            const bool originalEnabled = o_Renderer_get_enabled ? o_Renderer_get_enabled(renderer) : true;
            if (originalMaterial) enemyChamsRenderers.push_back({ renderer, originalMaterial, originalEnabled, 0, 0 });
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    sprintf_s(enemyChamsStatus, "Applied to %zu enemy renderer(s)", enemyChamsRenderers.size());
}

static bool ApplyEnemyChamsRendererMaterial(uintptr_t renderer, uintptr_t replacement,
    uintptr_t wallOverlay, bool throughWalls)
{
    if (!renderer || !replacement) return false;
    __try {
        if (throughWalls) {
            if (o_Renderer_set_enabled) o_Renderer_set_enabled(renderer, true);
            if (o_SkinnedMeshRenderer_set_updateWhenOffscreen)
                o_SkinnedMeshRenderer_set_updateWhenOffscreen(renderer, true);
            if (wallOverlay && o_Renderer_set_materials && g_il2cpp.array_new) {
                // X-ray is rendered first; selected Lit/Metallic/etc. renders after it
                // and replaces Flat wherever the enemy is directly visible.
                SetRendererMaterialPair(renderer, wallOverlay, replacement);
                return true;
            }
        }
        const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(renderer) : 0;
        if (currentMaterial != replacement) o_Renderer_set_material(renderer, replacement);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void RestoreEnemyOutlineOriginalMaterialUnsafe(uintptr_t renderer, uintptr_t originalMaterial)
{
    __try {
        if (renderer && originalMaterial && o_Renderer_get_material &&
            o_Renderer_get_material(renderer) != originalMaterial)
            o_Renderer_set_material(renderer, originalMaterial);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void UpdateEnemyChams()
{
    if (!keyValidated || !o_Renderer_set_material || !o_Material_set_color) return;
    if (enemyChamsThroughWallsObserved != enemyChamsThroughWalls) {
        enemyChamsThroughWallsObserved = enemyChamsThroughWalls;
        enemyChamsInitializedOcclusionControllers.clear();
    }
    if (!enemyChamsEnabled) {
        if (!enemyChamsRenderers.empty()) RestoreEnemyChams();
        strcpy_s(enemyChamsStatus, "Disabled");
        return;
    }

    std::vector<uintptr_t> liveRenderers;
    CollectEnemyBodyRenderers(liveRenderers);
    bool rendererSetChanged = liveRenderers.size() != enemyChamsRenderers.size();
    if (!rendererSetChanged) {
        for (size_t i = 0; i < liveRenderers.size(); ++i) {
            if (enemyChamsRenderers[i].renderer != liveRenderers[i]) { rendererSetChanged = true; break; }
        }
    }
    if (rendererSetChanged) CaptureEnemyChamsRenderers(liveRenderers);
    if (enemyChamsRenderers.empty()) { strcpy_s(enemyChamsStatus, "Waiting for enemy models"); return; }

    const uintptr_t replacement = EnsureSelectedEnemyChamsMaterial();
    if (!replacement) return;
    const uintptr_t wallOverlay = enemyChamsThroughWalls ? EnsureEnemyChamsMaterial(8) : 0;
    if (enemyChamsThroughWalls && !wallOverlay) return;
    if (enemyChamsMode == 3 && o_Material_SetFloat && g_il2cpp.string_new) {
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Metallic"), enemyChamsMetallic);
        o_Material_SetFloat(replacement, g_il2cpp.string_new("_Glossiness"), enemyChamsSmoothness);
    }
    if (o_Material_set_renderQueue)
        o_Material_set_renderQueue(replacement, enemyChamsThroughWalls ? 3999 :
            ((enemyChamsMode == 1 || enemyChamsMode == 4) ? 3000 : 2000));
    const Color displayColor = GetEnemyChamsDisplayColor();
    o_Material_set_color(replacement, displayColor);
    if (wallOverlay)
        o_Material_set_color(wallOverlay, Color(displayColor.r, displayColor.g, displayColor.b, 1.0f));
    size_t applied = 0;
    for (EnemyChamsRenderer& entry : enemyChamsRenderers) {
        if (enemyChamsMode == 7) {
            RestoreEnemyOutlineOriginalMaterialUnsafe(entry.renderer, entry.originalMaterial);
            if (!entry.outlineObject)
                CreateOutlineCloneUnsafe(entry.renderer, replacement, &entry.outlineObject, &entry.outlineRenderer);
            if (entry.outlineObject) ++applied;
        }
        else {
            DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
            if (ApplyEnemyChamsRendererMaterial(entry.renderer, replacement, wallOverlay, enemyChamsThroughWalls)) ++applied;
        }
    }
    sprintf_s(enemyChamsStatus, "Active on %zu enemy renderer(s)", applied);
}

static void AnimateEnemyChamsColor()
{
    if (!enemyChamsEnabled || (enemyChamsMode != 5 && enemyChamsMode != 6) || !o_Material_set_color) return;
    const ULONGLONG now = GetTickCount64();
    if (now - enemyChamsLastAnimationTick < 33) return;
    enemyChamsLastAnimationTick = now;
    const uintptr_t material = enemyChamsMaterials[enemyChamsMode];
    if (!material) return;
    __try { o_Material_set_color(material, GetEnemyChamsDisplayColor()); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
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
            if (originalMaterial) weaponChamsRenderers.push_back({ renderer, originalMaterial, 0, 0 });
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
    for (WeaponChamsRenderer& entry : weaponChamsRenderers) {
        if (!entry.renderer) continue;
        __try {
            if (weaponChamsMode == 7) {
                if (o_Renderer_get_material && o_Renderer_get_material(entry.renderer) != entry.originalMaterial)
                    o_Renderer_set_material(entry.renderer, entry.originalMaterial);
                if (!entry.outlineObject)
                    CreateOutlineCloneUnsafe(entry.renderer, replacement, &entry.outlineObject, &entry.outlineRenderer);
            }
            else {
                DestroyOutlineCloneUnsafe(&entry.outlineObject, &entry.outlineRenderer);
                const uintptr_t currentMaterial = o_Renderer_get_material ? o_Renderer_get_material(entry.renderer) : 0;
                if (currentMaterial != replacement) o_Renderer_set_material(entry.renderer, replacement);
            }
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
        localPlayerChamsRenderers.clear();
        enemyChamsRenderers.clear();
        enemyChamsInitializedOcclusionControllers.clear();
        weaponChamsController = 0;
        activeLocalWeaponController = 0;
        aimbotAutoFirePendingGun = 0;
        aimbotAutoFirePendingUntil = 0;
        aimbotAutoFireNextDecisionAt = 0;
        adminBhopObservedMovement = 0;
        adminBhopObservedJumpParameters = 0;
        adminBhopOriginalCaptured = false;
        armChamsArmsLodGroup = 0;
        gloveChamsArmsLodGroup = 0;
        localPlayerChamsLodGroup = 0;
        currentLocalCharacterLodGroupCache = 0;
        if (weaponChamsEnabled) InterlockedExchange(&pendingWeaponChamsRefresh, 1);
        if (armChamsEnabled) InterlockedExchange(&pendingArmChamsRefresh, 1);
        if (gloveChamsEnabled) InterlockedExchange(&pendingGloveChamsRefresh, 1);
        if (localPlayerChamsEnabled) InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
        if (enemyChamsEnabled) InterlockedExchange(&pendingEnemyChamsRefresh, 1);
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
    const bool localPlayerRefreshRequested = InterlockedExchange(&pendingLocalPlayerChamsRefresh, 0) != 0;
    const bool enemyRefreshRequested = InterlockedExchange(&pendingEnemyChamsRefresh, 0) != 0;
    const ULONGLONG chamsNow = GetTickCount64();
    const bool maintenanceDue = chamsNow - chamsLastMaintenanceTick >= 500;
    if (weaponRefreshRequested || armRefreshRequested || gloveRefreshRequested || localPlayerRefreshRequested || enemyRefreshRequested || maintenanceDue) {
        if (maintenanceDue) chamsLastMaintenanceTick = chamsNow;
        if (weaponRefreshRequested || (maintenanceDue && weaponChamsEnabled))
            UpdateWeaponChams(GetCurrentLocalWeaponController());
        const uintptr_t liveArmsLodGroup = GetCurrentLocalArmsLodGroup();
        if (armRefreshRequested || (maintenanceDue && armChamsEnabled)) UpdateArmChams(liveArmsLodGroup);
        if (gloveRefreshRequested || (maintenanceDue && gloveChamsEnabled)) UpdateGloveChams(liveArmsLodGroup);
        if (localPlayerRefreshRequested || (maintenanceDue && localPlayerChamsEnabled))
            UpdateLocalPlayerChams(GetCurrentLocalCharacterLodGroup());
        if (enemyRefreshRequested || (maintenanceDue && enemyChamsEnabled))
            UpdateEnemyChams();
    }

    AnimateWeaponChamsColor();
    AnimateArmChamsColor();
    AnimateGloveChamsColor();
    AnimateLocalPlayerChamsColor();
    AnimateEnemyChamsColor();
    AnimateWorldColor();
    if (adminBhopEnabled || adminBhopOriginalCaptured) ApplyAdminBhopState();
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

    const LONG fogCommand = InterlockedExchange(&pendingFogCommand, 0);
    const ULONGLONG fogNow = GetTickCount64();
    if (fogCommand == 2) RestoreOriginalFog();
    else if (keyValidated && fogEnabled && (fogCommand == 1 || fogNow - fogLastApplyTick >= 250)) {
        fogLastApplyTick = fogNow;
        ApplyFogSettings();
    }



    if (keyValidated && jbActive) {
        pixelSurfCharacterController = instance;
        float speed = sqrt(motion.x * motion.x + motion.z * motion.z);
        if (speed > 0.1f) {
            motion.x = (motion.x / speed) * surfSpeed * 10.0f;
            motion.z = (motion.z / speed) * surfSpeed * 10.0f;
        }

        motion.y = 0;
    }
    else if (keyValidated) {
        const LONG64 now = static_cast<LONG64>(GetTickCount64());
        const LONG64 releaseGraceUntil = InterlockedCompareExchange64(
            &pixelSurfReleaseGraceUntil, 0, 0);
        const LONG64 releasedAt = InterlockedCompareExchange64(
            &pixelSurfReleasedAt, 0, 0);
        if (pixelSurfReleaseHop && releaseGraceUntil > now && motion.y < 0.0f) {
            motion.y = 0.0f;
        }
        else if (!pixelSurfReleaseHop && releasedAt > 0 && now >= releasedAt &&
            instance == pixelSurfCharacterController && motion.y < 0.0f) {
            // Normal release: never fake grounded. Keep replacing the stale accumulated
            // gravity until this exact controller actually lands; do not expire mid-air.
            float deltaTime = o_Time_get_deltaTime ? o_Time_get_deltaTime() : (1.0f / 60.0f);
            if (!isfinite(deltaTime) || deltaTime < (1.0f / 240.0f) || deltaTime > 0.1f)
                deltaTime = 1.0f / 60.0f;
            const float elapsed = static_cast<float>(now - releasedAt) * 0.001f;
            float normalDownSpeed = 0.5f + 4.0f * elapsed;
            if (normalDownSpeed > 8.0f) normalDownSpeed = 8.0f;
            const float normalDownMotion = -normalDownSpeed * deltaTime;
            if (motion.y < normalDownMotion) motion.y = normalDownMotion;
        }
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

void DrawBulletImpacts()
{
    if (!keyValidated || !bulletImpactsEnabled || !o_WorldToScreenPoint) return;
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG durationMs = static_cast<ULONGLONG>(bulletImpactsDuration * 1000.0f);
    std::vector<BulletImpactEntry> snapshot;
    AcquireSRWLockExclusive(&bulletImpactLock);
    bulletImpacts.erase(std::remove_if(bulletImpacts.begin(), bulletImpacts.end(),
        [now, durationMs](const BulletImpactEntry& impact) {
            return now - impact.createdAt > durationMs;
        }), bulletImpacts.end());
    snapshot = bulletImpacts;
    ReleaseSRWLockExclusive(&bulletImpactLock);

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    for (const BulletImpactEntry& impact : snapshot) {
        ImVec2 screen;
        if (!ProjectTracerEnd(impact.position, screen)) continue;
        const float age = static_cast<float>(now - impact.createdAt) /
            (bulletImpactsDuration * 1000.0f);
        const float alpha = 1.0f - (age < 0.0f ? 0.0f : (age > 1.0f ? 1.0f : age));
        const float half = bulletImpactsSize * 0.5f;
        // CS2-style solid 3D impact cube: filled faces only, with no outline
        // border. Slight face shading keeps its volume readable while the inside
        // remains completely filled.
        const float* selectedColor = impact.serverConfirmed ?
            bulletImpactsConfirmedColor : bulletImpactsClientColor;
        const int baseR = static_cast<int>(selectedColor[0] * 255.0f);
        const int baseG = static_cast<int>(selectedColor[1] * 255.0f);
        const int baseB = static_cast<int>(selectedColor[2] * 255.0f);
        Vector3 corners[8];
        ImVec2 projected[8];
        bool allProjected = true;
        for (int corner = 0; corner < 8; ++corner) {
            corners[corner] = Vector3(
                impact.position.x + ((corner & 1) ? half : -half),
                impact.position.y + ((corner & 2) ? half : -half),
                impact.position.z + ((corner & 4) ? half : -half));
            if (!ProjectTracerEnd(corners[corner], projected[corner])) allProjected = false;
        }
        if (!allProjected) continue;
        static const int faces[6][4] = {
            {0,2,3,1}, {4,5,7,6}, {0,1,5,4},
            {2,6,7,3}, {0,4,6,2}, {1,3,7,5}
        };
        static const float faceShade[6] = { 0.78f, 1.00f, 0.88f, 0.70f, 0.82f, 0.94f };
        for (int face = 0; face < 6; ++face) {
            ImVec2 quad[4] = {
                projected[faces[face][0]], projected[faces[face][1]],
                projected[faces[face][2]], projected[faces[face][3]]
            };
            const float shade = faceShade[face];
            const ImU32 faceColor = IM_COL32(
                static_cast<int>(baseR * shade),
                static_cast<int>(baseG * shade),
                static_cast<int>(baseB * shade),
                static_cast<int>(255.0f * alpha));
            draw->AddConvexPolyFilled(quad, 4, faceColor);
        }
    }
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

            if (jbBind.key > 0) {

                bool held = (GetAsyncKeyState(jbBind.key) & 0x8000) != 0;

                static bool jbToggle = false, jbLast = false;

                bool requestedPixelSurf = held;
                if (jbBind.toggleMode) {
                    if (held && !jbLast) jbToggle = !jbToggle;
                    requestedPixelSurf = jbToggle;
                }
                SetPixelSurfActive(requestedPixelSurf);

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
    RefreshConfigFiles(); // Creates Documents\ze0nware immediately.

}



static bool IsMouseOrRawInputMessage(UINT message)
{
    switch (message) {
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        return true;
    default:
        return false;
    }
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (uMsg == WM_KEYUP && wParam == VK_INSERT) {
        menuOpen = !menuOpen;
        if (menuOpen) {
            ReleaseCapture();
            ClipCursor(nullptr);
        } else {
            showConfigMenu = false;
            waitingForBind = nullptr;
        }
        return true;
    }

    bool imguiHandled = false;
    if (keyValidated && menuOpen && ImGui::GetCurrentContext())
        imguiHandled = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) != 0;

    if (keyValidated && menuOpen && waitingForBind &&
        (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN || uMsg == WM_LBUTTONDOWN ||
         uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)) {
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

    // Feed ImGui first, then prevent Unity from receiving any mouse/raw input while
    // the overlay is open. This blocks camera movement, firing, aiming and wheel input.
    if (keyValidated && menuOpen && IsMouseOrRawInputMessage(uMsg)) return 0;
    if (keyValidated && menuOpen && imguiHandled) return true;

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
    ImGui::GetIO().MouseDrawCursor = keyValidated && menuOpen;







    UpdateTrail();
    DrawTrail();

    

    // Render ESP if enabled

    if (boxEsp) {

        BoxEsp();

    }



    // Menu windows are rendered only while Insert toggle is open.
    if (menuOpen || menuOpenAnimation > 0.0f) {



    ImGuiStyle& style = ImGui::GetStyle();
    const float menuTarget = menuOpen ? 1.0f : 0.0f;
    const float menuStep = 1.0f - expf(-ImGui::GetIO().DeltaTime * 13.0f);
    menuOpenAnimation += (menuTarget - menuOpenAnimation) * menuStep;
    if (menuOpenAnimation < 0.002f) menuOpenAnimation = 0.0f;
    if (menuOpenAnimation > 0.998f) menuOpenAnimation = 1.0f;

    if (menuOpenAnimation > 0.0f) {
    // Neverlose-inspired shell: dense near-black navy surfaces, restrained cyan
    // accent, compact controls and a persistent left navigation rail.
    const float easedOpen = menuOpenAnimation * menuOpenAnimation * (3.0f - 2.0f * menuOpenAnimation);
    style.Alpha = easedOpen;
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ScrollbarSize = 7.0f;
    style.GrabMinSize = 8.0f;
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.WindowPadding = ImVec2(0.0f, 0.0f);
    style.FramePadding = ImVec2(7.0f, 4.0f);

    // Exact Figma scheme: #0e0e0e shell, #111111 panels, #161616 selected tab,
    // #ca7384 accent, #eaeaea text and black one-pixel borders.
    const ImVec4 accent(202.0f / 255.0f, 115.0f / 255.0f, 132.0f / 255.0f, 1.0f);
    const ImVec4 accentDim(157.0f / 255.0f, 103.0f / 255.0f, 113.0f / 255.0f, 1.0f);
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(234.0f / 255.0f, 234.0f / 255.0f, 234.0f / 255.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(234.0f / 255.0f, 234.0f / 255.0f, 234.0f / 255.0f, 0.40f);
    colors[ImGuiCol_WindowBg] = ImVec4(14.0f / 255.0f, 14.0f / 255.0f, 14.0f / 255.0f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(27.0f / 255.0f, 27.0f / 255.0f, 27.0f / 255.0f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(27.0f / 255.0f, 27.0f / 255.0f, 27.0f / 255.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(17.0f / 255.0f, 17.0f / 255.0f, 17.0f / 255.0f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 0.50f);
    colors[ImGuiCol_ButtonActive] = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 0.50f);
    colors[ImGuiCol_HeaderActive] = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 1.0f);
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentDim;
    colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(14.0f / 255.0f, 14.0f / 255.0f, 14.0f / 255.0f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(27.0f / 255.0f, 27.0f / 255.0f, 27.0f / 255.0f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = accentDim;
    colors[ImGuiCol_ScrollbarGrabActive] = accent;

    // Fit smaller resolutions instead of letting the bottom/right controls leave the screen.
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const ImVec2 fullSize(
        (display.x - 24.0f < 740.0f) ? display.x - 24.0f : 740.0f,
        (display.y - 24.0f < 532.0f) ? display.y - 24.0f : 532.0f);
    const float scale = 0.975f + 0.025f * easedOpen;
    const ImVec2 animatedSize(fullSize.x * scale, fullSize.y * scale);
    static ImVec2 menuPosition(0.0f, 0.0f);
    static ImVec2 lastDisplaySize(0.0f, 0.0f);
    static bool menuPositionInitialized = false;
    if (!menuPositionInitialized || display.x != lastDisplaySize.x || display.y != lastDisplaySize.y) {
        menuPosition = ImVec2((display.x - animatedSize.x) * 0.5f,
            (display.y - animatedSize.y) * 0.5f);
        lastDisplaySize = display;
        menuPositionInitialized = true;
    }
    // Clamp after resolution changes and after dragging near an edge.
    if (menuPosition.x < 8.0f) menuPosition.x = 8.0f;
    if (menuPosition.y < 8.0f) menuPosition.y = 8.0f;
    if (menuPosition.x + animatedSize.x > display.x - 8.0f)
        menuPosition.x = display.x - animatedSize.x - 8.0f;
    if (menuPosition.y + animatedSize.y > display.y - 8.0f)
        menuPosition.y = display.y - animatedSize.y - 8.0f;
    ImGui::SetNextWindowPos(menuPosition, ImGuiCond_Always);
    ImGui::SetNextWindowSize(animatedSize, ImGuiCond_Always);
    ImGui::Begin("ze0nware##main", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        (menuOpen ? 0 : ImGuiWindowFlags_NoInputs));

    ImDrawList* shellDraw = ImGui::GetWindowDrawList();
    const ImVec2 wp = ImGui::GetWindowPos();
    const ImVec2 ws = ImGui::GetWindowSize();
    const ImVec2 wmax(wp.x + ws.x, wp.y + ws.y);
    // Exact outer shell from the Figma file: flat #0e0e0e with a black border.
    shellDraw->AddRect(wp, wmax, ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)),
        0.0f, 0, 1.0f);

    // Full-width 31px title/drag bar, matching the Figma main frame offset.
    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    ImGui::InvisibleButton("##MenuTitleDrag", ImVec2(animatedSize.x, 31.0f));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        menuPosition.x += ImGui::GetIO().MouseDelta.x;
        menuPosition.y += ImGui::GetIO().MouseDelta.y;
        if (menuPosition.x < 8.0f) menuPosition.x = 8.0f;
        if (menuPosition.y < 8.0f) menuPosition.y = 8.0f;
        if (menuPosition.x + animatedSize.x > display.x - 8.0f)
            menuPosition.x = display.x - animatedSize.x - 8.0f;
        if (menuPosition.y + animatedSize.y > display.y - 8.0f)
            menuPosition.y = display.y - animatedSize.y - 8.0f;
        ImGui::SetWindowPos("ze0nware##main", menuPosition, ImGuiCond_Always);
    }
    ImGui::SetCursorPos(ImVec2(10.0f, 31.0f));
    ImGui::BeginChild("FigmaMain", ImVec2(animatedSize.x - 20.0f, animatedSize.y - 41.0f), true);

    // Persistent left navigation rail, as used by the Neverlose layout.
    static int currentTab = 0;
    ImGui::BeginChild("Sidebar", ImVec2(140.0f, 0.0f), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPos(ImVec2(18.0f, 18.0f));
    ImGui::TextColored(accent, "ze0nware");
    ImGui::SetCursorPosY(93.0f);

    const char* navLabels[] = { "AIMBOT", "ANTI-AIM", "VISUALS", "WEAPONS", "CONFIGS" };
    for (int tab = 0; tab < 5; ++tab) {
        const bool selected = currentTab == tab;
        ImGui::SetCursorPosX(3.0f);
        ImGui::PushID(tab);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 22.0f / 255.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.96f, 1.0f, 1.0f));
        }
        const ImVec2 buttonPos = ImGui::GetCursorScreenPos();
        if (ImGui::Button(navLabels[tab], ImVec2(134.0f, 32.0f))) currentTab = tab;
        if (selected) {
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(buttonPos.x, buttonPos.y), ImVec2(buttonPos.x + 3.0f, buttonPos.y + 32.0f),
                ImGui::GetColorU32(accent), 2.0f);
            ImGui::PopStyleColor(3);
        }
        ImGui::PopID();
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
    }

    ImGui::EndChild();
    ImGui::SameLine(0.0f, 0.0f);

    // Main content area: compact two-column section cards on a dark navy surface.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
    ImGui::BeginChild("ContentPanel", ImVec2(0, 0), true,
        ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::PopStyleVar();
    
    if (currentTab == 0) { // AIMBOT / Movement
        ImGui::Columns(2, nullptr, false);
        
        ImGui::BeginChild("Movement", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Movement");
        ImGui::Separator();
        ImGui::Spacing();
        
        bool pixelSurfRequested = jbActive;
        if (ImGui::Checkbox("Pixel Surf", &pixelSurfRequested))
            SetPixelSurfActive(pixelSurfRequested);
        if (jbActive) {
            ImGui::SliderFloat("Pixel Surf Speed", &surfSpeed, 0.5f, 3.0f, "%.2f");
        }
        ImGui::Checkbox("Release Hop", &pixelSurfReleaseHop);
        if (ImGui::Checkbox("Admin Panel BHop", &adminBhopEnabled)) ApplyAdminBhopState();
        if (adminBhopEnabled) {
            if (ImGui::SliderFloat("Admin BHop Speed", &adminBhopMaxSpeed,
                1.0f, 30.0f, "%.2f"))
                ApplyAdminBhopState();
            if (ImGui::Checkbox("CS Manual Air Strafe", &adminBhopCsStrafeMode))
                ApplyAdminBhopState();
            if (adminBhopCsStrafeMode)
                ImGui::TextWrapped("A + left / D + right. W is ignored; S works for backward BHop. Speed controls acceleration and max speed.");
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
        if (aimbotVisibleCheck) {
            ImGui::Checkbox("Auto Wall", &aimbotAutoWall);
            if (aimbotAutoWall)
                ImGui::SliderFloat("Auto Wall Min Damage", &aimbotAutoWallMinDamage,
                    1.0f, 100.0f, "%.0f");
        }
        ImGui::Checkbox("Auto Fire", &aimbotAutoFire);
        ImGui::Text("FOV: %.0f degrees", aimbotFov);
        ImGui::Spacing();
        ImGui::TextColored(accent, "Keybinds");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Text("Pixel Surf Key");
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
        if (ImGui::Checkbox("Command Anti-Aim", &silentAntiAimEnabled)) {
            InterlockedExchange(&silentAntiAimInputBuildCalls, 0);
            InterlockedExchange(&silentAntiAimCommandCalls, 0);
            InterlockedExchange(&silentAntiAimAppliedCalls, 0);
            InterlockedExchange(&silentAntiAimCameraRestoreCalls, 0);
            silentAntiAimLatestInputValid = false;
            silentAntiAimLatestFiring = false;
            silentAntiAimLatestPlayer = 0;
            silentAntiAimLatestController = 0;
            silentAntiAimRealCameraValid = false;
            silentAntiAimLastSpinTick = 0;
            silentAntiAimSpinYaw = 0.0f;
            strcpy_s(silentAntiAimStatus, !silentAntiAimEnabled ? "Disabled" :
                (silentAntiAimHookReady ? "Command hook active" :
                                         "Command anti-aim hooks failed"));
        }
        ImGui::Spacing();
        const char* antiAimModes[] = { "Static", "Jitter", "Spin" };
        ImGui::SetNextItemWidth(190.0f);
        ImGui::Combo("Mode", &silentAntiAimMode, antiAimModes, 3);
        const char* pitchModes[] = { "Neutral", "Down", "Up" };
        ImGui::SetNextItemWidth(190.0f);
        ImGui::Combo("Pitch", &silentAntiAimPitch, pitchModes, 3);
        if (silentAntiAimMode != 2) {
            ImGui::SetNextItemWidth(190.0f);
            ImGui::SliderFloat("Yaw", &silentAntiAimYaw,
                -180.0f, 180.0f, "%.0f deg");
        }
        if (silentAntiAimMode == 1) {
            ImGui::SetNextItemWidth(190.0f);
            ImGui::SliderFloat("Jitter Range", &silentAntiAimJitterRange,
                0.0f, 180.0f, "%.0f deg");
        }
        if (silentAntiAimMode == 2) {
            ImGui::SetNextItemWidth(190.0f);
            ImGui::SliderFloat("Spin Speed", &silentAntiAimSpinSpeed,
                1.0f, 1080.0f, "%.0f deg/s");
        }

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
        }
        ImGui::Checkbox("Show Velocity", &showVelocity);
        ImGui::Checkbox("Show Trail", &showTrail);
        if (ImGui::Checkbox("Freeze Corpses", &freezeCorpsesEnabled) && !freezeCorpsesEnabled)
            UpdateFrozenCorpses();
        if (freezeCorpsesEnabled) {
            ImGui::SliderFloat("Corpse Duration", &freezeCorpsesDuration, 0.5f, 10.0f, "%.1f s");
            ImGui::Checkbox("Corpse Fade Out", &freezeCorpsesFadeEnabled);
            if (freezeCorpsesFadeEnabled)
                ImGui::SliderFloat("Corpse Fade Duration", &freezeCorpsesFadeDuration,
                    0.1f, 4.0f, "%.1f s");
            const char* corpseChamsModes[] = { "None", "Flat", "Glass", "Lit", "Metallic", "Rainbow", "Pulse", "Outline" };
            ImGui::Combo("Corpse Chams", &freezeCorpsesChamsMode,
                corpseChamsModes, IM_ARRAYSIZE(corpseChamsModes));
            if (freezeCorpsesChamsMode != 0)
                ImGui::ColorEdit3("Corpse Color", freezeCorpsesChamsColor);
            if (freezeCorpsesChamsMode == 2)
                ImGui::SliderFloat("Corpse Glass Alpha", &freezeCorpsesChamsAlpha, 0.05f, 0.95f, "%.2f");
            if (freezeCorpsesChamsMode == 4) {
                ImGui::SliderFloat("Corpse Metallic", &freezeCorpsesMetallic, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Corpse Smoothness", &freezeCorpsesSmoothness, 0.0f, 1.0f, "%.2f");
            }
            if (freezeCorpsesChamsMode == 5 || freezeCorpsesChamsMode == 6)
                ImGui::SliderFloat("Corpse Animation Speed", &freezeCorpsesAnimationSpeed, 0.1f, 5.0f, "%.2f");
        }
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
        }
        if (ImGui::Checkbox("Penetrable Surfaces", &penetrableSurfacesEnabled)) {
            if (!penetrableSurfacesEnabled) RestorePenetrableSurfaceVisualization();
            penetrableSurfaceNextScan = 0;
        }
        if (penetrableSurfacesEnabled) {
            ImGui::TextDisabled("360-degree map scan using native SurfaceType tags");
            ImGui::ColorEdit3("Penetrable Fill", penetrableSurfaceFillColor);
            ImGui::TextDisabled("No enemy or shot required; excludes solid/character surfaces");
            ImGui::SliderFloat("Penetrable Transparency", &penetrableSurfaceAlpha, 0.05f, 0.80f, "%.2f");
            ImGui::SliderFloat("Penetrable World Radius", &penetrableSurfaceScanDistance, 10.0f, 250.0f, "%.0f m");
            ImGui::TextDisabled("%s", penetrableSurfaceStatus);
        }
        if (ImGui::Checkbox("Third Person", &thirdPersonEnabled)) {
            strcpy_s(thirdPersonStatus, thirdPersonEnabled ? "TPS transition queued" : "FPS transition queued");
            InterlockedExchange(&pendingThirdPersonCommand, 1);
        }
        if (thirdPersonEnabled) {
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
        if (ImGui::Checkbox("Fog", &fogEnabled))
            InterlockedExchange(&pendingFogCommand, fogEnabled ? 1 : 2);
        if (fogEnabled) {
            bool fogChanged = ImGui::ColorEdit3("Fog Color", fogColor);
            if (ImGui::SliderFloat("Fog Start Distance", &fogStartDistance, 0.0f, 500.0f, "%.1f m")) {
                if (fogStartDistance > fogEndDistance) fogEndDistance = fogStartDistance;
                fogChanged = true;
            }
            if (ImGui::SliderFloat("Fog End Distance", &fogEndDistance, 1.0f, 1000.0f, "%.1f m")) {
                if (fogEndDistance < fogStartDistance) fogStartDistance = fogEndDistance;
                fogChanged = true;
            }
            if (ImGui::SliderFloat("Fog Density", &fogDensity, 0.0f, 0.20f, "%.4f")) fogChanged = true;
            if (fogChanged) InterlockedExchange(&pendingFogCommand, 1);
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
        
        ImGui::EndChild();
    }
    else if (currentTab == 3) { // WEAPONS
        ImGui::BeginChild("Weapons", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Weapons");
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Checkbox("Double Tap", &doubleTapEnabled))
            InterlockedExchange(&doubleTapExtraShots, 0);
        if (ImGui::Checkbox("Infinity Ammo", &infinityAmmo)) {
            frozenAmmoWeapon = 0;
            frozenAmmoValue = -1;
            InterlockedExchange(&infinityAmmoFireCalls, 0);
            InterlockedExchange(&infinityAmmoGetterCalls, 0);
            InterlockedExchange(&infinityAmmoRestores, 0);
        }
        ImGui::Checkbox("No Spread", &noSpreadEnabled);
        if (ImGui::Checkbox("Remove Scope Borders", &removeScopeBorders)) InterlockedExchange(&pendingScopeOverlayRefresh, 1);
        if (ImGui::Checkbox("Weapon Chams", &weaponChamsEnabled)) {
            strcpy_s(weaponChamsStatus, weaponChamsEnabled ? "Applying to equipped weapon" : "Restoring original materials");
            InterlockedExchange(&pendingWeaponChamsRefresh, 1);
        }
        if (weaponChamsEnabled) {
            const char* chamsModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
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
        }
        if (ImGui::Checkbox("Arm Chams", &armChamsEnabled)) {
            strcpy_s(armChamsStatus, armChamsEnabled ? "Applying to arms" : "Restoring original arm material");
            InterlockedExchange(&pendingArmChamsRefresh, 1);
        }
        if (armChamsEnabled) {
            const char* handModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
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
        }
        if (ImGui::Checkbox("Glove Chams", &gloveChamsEnabled)) {
            strcpy_s(gloveChamsStatus, gloveChamsEnabled ? "Applying to gloves" : "Restoring original glove material");
            InterlockedExchange(&pendingGloveChamsRefresh, 1);
        }
        if (gloveChamsEnabled) {
            const char* gloveModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
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
        }
        if (ImGui::Checkbox("Local Player Chams", &localPlayerChamsEnabled)) {
            InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
        }
        if (localPlayerChamsEnabled) {
            const char* localModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
            if (ImGui::Combo("Local Player Material", &localPlayerChamsMode, localModes, IM_ARRAYSIZE(localModes)))
                InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
            if (ImGui::ColorEdit3("Local Player Color", localPlayerChamsColor))
                InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
            if ((localPlayerChamsMode == 1 || localPlayerChamsMode == 4) &&
                ImGui::SliderFloat("Local Player Transparency", &localPlayerChamsAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
            if (localPlayerChamsMode == 3) {
                if (ImGui::SliderFloat("Local Player Metallic", &localPlayerChamsMetallic, 0.0f, 1.0f, "%.2f"))
                    InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
                if (ImGui::SliderFloat("Local Player Smoothness", &localPlayerChamsSmoothness, 0.0f, 1.0f, "%.2f"))
                    InterlockedExchange(&pendingLocalPlayerChamsRefresh, 1);
            }
            if (localPlayerChamsMode == 5 || localPlayerChamsMode == 6)
                ImGui::SliderFloat("Local Player Animation Speed", &localPlayerChamsAnimationSpeed, 0.2f, 5.0f, "%.1f");
        }
        if (ImGui::Checkbox("Enemy Chams", &enemyChamsEnabled)) {
            InterlockedExchange(&pendingEnemyChamsRefresh, 1);
        }
        if (enemyChamsEnabled) {
            const char* enemyModes[] = { "Flat", "Glass", "Lit", "Metallic", "Transparent Lit", "Rainbow", "Pulse", "Outline" };
            if (ImGui::Combo("Enemy Material", &enemyChamsMode, enemyModes, IM_ARRAYSIZE(enemyModes)))
                InterlockedExchange(&pendingEnemyChamsRefresh, 1);
            if (ImGui::Checkbox("Through Walls", &enemyChamsThroughWalls))
                InterlockedExchange(&pendingEnemyChamsRefresh, 1);
            if (ImGui::ColorEdit3("Enemy Color", enemyChamsColor))
                InterlockedExchange(&pendingEnemyChamsRefresh, 1);
            if ((enemyChamsMode == 1 || enemyChamsMode == 4) &&
                ImGui::SliderFloat("Enemy Transparency", &enemyChamsAlpha, 0.05f, 0.95f, "%.2f"))
                InterlockedExchange(&pendingEnemyChamsRefresh, 1);
            if (enemyChamsMode == 3) {
                if (ImGui::SliderFloat("Enemy Metallic", &enemyChamsMetallic, 0.0f, 1.0f, "%.2f"))
                    InterlockedExchange(&pendingEnemyChamsRefresh, 1);
                if (ImGui::SliderFloat("Enemy Smoothness", &enemyChamsSmoothness, 0.0f, 1.0f, "%.2f"))
                    InterlockedExchange(&pendingEnemyChamsRefresh, 1);
            }
            if (enemyChamsMode == 5 || enemyChamsMode == 6)
                ImGui::SliderFloat("Enemy Animation Speed", &enemyChamsAnimationSpeed, 0.2f, 5.0f, "%.1f");
        }
        if (ImGui::Checkbox("Hit Log", &hitLogEnabled)) {
            AcquireSRWLockExclusive(&hitLogLock);
            hitLogEntries.clear();
            hitLogLastResult = 0;
            hitLogLastResultAt = 0;
            ReleaseSRWLockExclusive(&hitLogLock);
        }
        if (hitLogEnabled)
            ImGui::SliderFloat("Hit Log Duration", &hitLogDuration, 1.0f, 8.0f, "%.1f s");
        ImGui::Checkbox("Hit Marker", &hitMarkerEnabled);
        if (hitMarkerEnabled) {
            ImGui::ColorEdit3("Hit Marker Color", hitMarkerColor);
            ImGui::SliderFloat("Hit Marker Duration", &hitMarkerDuration, 0.08f, 1.0f, "%.2f s");
            ImGui::SliderFloat("Hit Marker Size", &hitMarkerSize, 3.0f, 24.0f, "%.0f px");
            ImGui::SliderFloat("Hit Marker Gap", &hitMarkerGap, 0.0f, 16.0f, "%.0f px");
            ImGui::SliderFloat("Hit Marker Thickness", &hitMarkerThickness, 1.0f, 6.0f, "%.1f px");
        }
        if (ImGui::Checkbox("Bullet Impacts", &bulletImpactsEnabled) && !bulletImpactsEnabled) {
            AcquireSRWLockExclusive(&bulletImpactLock);
            bulletImpacts.clear();
            ReleaseSRWLockExclusive(&bulletImpactLock);
        }
        if (bulletImpactsEnabled) {
            ImGui::TextDisabled("Client/penetration and confirmed-hit colors");
            ImGui::ColorEdit3("Client Impact Color", bulletImpactsClientColor);
            ImGui::ColorEdit3("Confirmed Impact Color", bulletImpactsConfirmedColor);
            ImGui::SliderFloat("Impact Duration", &bulletImpactsDuration, 0.25f, 10.0f, "%.1f s");
            ImGui::SliderFloat("Impact 3D Size", &bulletImpactsSize, 0.02f, 0.30f, "%.2f u");
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
    else if (currentTab == 4) { // CONFIGS
        ImGui::BeginChild("Configs", ImVec2(0, 0), true);
        ImGui::TextColored(accent, "Configs");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetNextItemWidth(240.0f);
        ImGui::InputText("Name", configName, sizeof(configName));
        if (ImGui::Button("Save", ImVec2(100.0f, 0.0f))) SaveConfig(configName);
        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(100.0f, 0.0f))) LoadConfig(configName);
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(100.0f, 0.0f))) DeleteConfig(configName);
        ImGui::Spacing();
        if (ImGui::Button("Refresh")) RefreshConfigFiles();
        ImGui::Separator();
        ImGui::BeginChild("ConfigList", ImVec2(0.0f, 0.0f), true);
        for (const std::string& saved : configFiles) {
            const bool selected = saved == configName;
            if (ImGui::Selectable(saved.c_str(), selected))
                strcpy_s(configName, saved.c_str());
        }
        ImGui::EndChild();
        ImGui::TextDisabled("%s", configStatus);
        ImGui::EndChild();
    }
    
    ImGui::EndChild(); // ContentPanel
    ImGui::EndChild(); // FigmaMain
    ImGui::End();
    style.Alpha = 1.0f;
    } // animated menu

    } // menuOpen or fade-out



    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    if (jbActive) {

        ImVec2 ts = ImGui::CalcTextSize("ps");

        float x = (ImGui::GetIO().DisplaySize.x - ts.x) * 0.5f, y = ImGui::GetIO().DisplaySize.y - 100.0f;

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x + 2, y + 2), IM_COL32(0, 0, 0, 160), "ps");

        ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), IM_COL32(100, 255, 140, 255), "ps");

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
    DrawHitLog();
    DrawBulletImpacts();
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

    // IL2CPP init: locate dump1 classes and singleton backing fields
        bool __ok_init = g_il2cpp.init();
    LINDY_LOG("[init] g_il2cpp.init=%d ga=%p", (int)__ok_init, (void*)g_il2cpp.ga);
    if (__ok_init) {
        g_GameControllerClass = g_il2cpp.find_class("Chillow.StandChillow.Game", "GameController");
        if (g_GameControllerClass)
            g_GameControllerInstanceField = g_il2cpp.class_get_field_from_name(g_GameControllerClass, "<chmn>k__BackingField");
        g_PlayerManagerClass = g_il2cpp.find_class("Chillow.StandChillow.Player", "PlayerManager");
        if (!g_PlayerManagerClass) g_PlayerManagerClass = g_il2cpp.find_class("", "PlayerManager");
        LINDY_LOG("[init] PlayerManagerClass=%p", (void*)g_PlayerManagerClass);
        if (g_PlayerManagerClass) {
            Il2CppClass* playerControllerClass = g_il2cpp.find_class("Chillow.StandChillow.Player", "PlayerController");
            if (!playerControllerClass) playerControllerClass = g_il2cpp.find_class("", "PlayerController");
            if (playerControllerClass && g_il2cpp.class_get_type && g_il2cpp.type_get_object) {
                const Il2CppType* playerType = g_il2cpp.class_get_type(playerControllerClass);
                g_PlayerControllerReflectionType = playerType ? g_il2cpp.type_get_object(playerType) : nullptr;
            }
            LINDY_LOG("[init] PlayerController reflection type=%p", (void*)g_PlayerControllerReflectionType);
            // dump1 LazySingleton<T> stores the instance in parent field ckjy.
            Il2CppClass* parent = g_il2cpp.class_get_parent ? g_il2cpp.class_get_parent(g_PlayerManagerClass) : nullptr;
            if (parent) {
                g_PlayerManagerInstanceField = g_il2cpp.class_get_field_from_name(parent, "ckjy");
            }
            if (!g_PlayerManagerInstanceField) {
                // fallback: try on class itself
                g_PlayerManagerInstanceField = g_il2cpp.class_get_field_from_name(g_PlayerManagerClass, "ckjy");
            }
        }
        g_GlovesManagerClass = g_il2cpp.find_class("Chillow.StandChillow.Main.Inventory.Gloves", "GlovesManager");
        LINDY_LOG("[init] GlovesManagerClass=%p", (void*)g_GlovesManagerClass);
        if (g_GlovesManagerClass) {
            // dump1 Singleton<T> stores the active instance in parent field ckky.
            Il2CppClass* parent = g_il2cpp.class_get_parent ? g_il2cpp.class_get_parent(g_GlovesManagerClass) : nullptr;
            if (parent) g_GlovesManagerInstanceField = g_il2cpp.class_get_field_from_name(parent, "ckky");
            if (!g_GlovesManagerInstanceField)
                g_GlovesManagerInstanceField = g_il2cpp.class_get_field_from_name(g_GlovesManagerClass, "ckky");
        }
    }
    base = (uintptr_t)GetModuleHandleA("GameAssembly.dll");

    unityPlayerBase = (uintptr_t)GetModuleHandleA("UnityPlayer.dll");

    if (!base || !unityPlayerBase) return 0;

    

    g_MatrixValid = true;



    o_CheatRuntime_SetThirdPerson = (void(__fastcall*)(uintptr_t, bool, const Il2CppMethod*))(base + OFFSET_CHEAT_RUNTIME_SET_THIRDPERSON);
    o_CheatRuntime_SetBhop = (void(__fastcall*)(uintptr_t, uintptr_t, bool))(base + OFFSET_CHEAT_RUNTIME_SET_BHOP);
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
    o_Transform_get_localEulerAngles = (Vector3(__fastcall*)(uintptr_t))(base + OFFSET_TRANSFORM_GET_LOCALEULERANGLES);
    o_Transform_set_localEulerAngles = (void(__fastcall*)(uintptr_t, Vector3))(base + OFFSET_TRANSFORM_SET_LOCALEULERANGLES);

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
    o_Renderer_get_enabled = (bool(__fastcall*)(uintptr_t))(base + OFFSET_RENDERER_GET_ENABLED);
    o_Renderer_set_enabled = (void(__fastcall*)(uintptr_t, bool))(base + OFFSET_RENDERER_SET_ENABLED);
    o_SkinnedMeshRenderer_set_updateWhenOffscreen = (void(__fastcall*)(uintptr_t, bool))(base + OFFSET_SKINNEDMESHRENDERER_SET_UPDATE_WHEN_OFFSCREEN);
    const MH_STATUS enemyOcclusionCreate = MH_CreateHook(
        (LPVOID)(base + OFFSET_OBJECT_OCCLUDEE_SET_VISIBLE_STATE),
        hk_ObjectOccludee_SetVisibleState,
        (LPVOID*)&o_ObjectOccludee_SetVisibleState);
    const MH_STATUS enemyOcclusionEnable = enemyOcclusionCreate == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_OBJECT_OCCLUDEE_SET_VISIBLE_STATE)) : enemyOcclusionCreate;
    LINDY_LOG("[EnemyChams] occlusion visibility hook create=%d enable=%d",
        (int)enemyOcclusionCreate, (int)enemyOcclusionEnable);
    o_Object_FindObjectsOfType = (Il2CppArray*(__fastcall*)(Il2CppObject*, bool, const Il2CppMethod*))(base + OFFSET_OBJECT_FIND_OBJECTS_OF_TYPE);
    o_Material_get_color = (Color(__fastcall*)(uintptr_t))(base + OFFSET_MATERIAL_GET_COLOR);
    o_Material_set_color = (void(__fastcall*)(uintptr_t, Color))(base + OFFSET_MATERIAL_SET_COLOR);
    o_Material_set_renderQueue = (void(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_SET_RENDERQUEUE);
    o_Material_SetFloat = (void(__fastcall*)(uintptr_t, Il2CppString*, float))(base + OFFSET_MATERIAL_SET_FLOAT);
    o_Material_HasProperty = (bool(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_HAS_PROPERTY);
    o_Material_GetColorId = (Color(__fastcall*)(uintptr_t, int))(base + OFFSET_MATERIAL_GET_COLOR_ID);
    o_Material_SetColorId = (void(__fastcall*)(uintptr_t, int, Color))(base + OFFSET_MATERIAL_SET_COLOR_ID);
    o_Shader_PropertyToID = (int(__fastcall*)(Il2CppString*, const Il2CppMethod*))(base + OFFSET_SHADER_PROPERTY_TO_ID);
    o_RenderSettings_get_fog = (bool(__fastcall*)(const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_FOG);
    o_RenderSettings_get_fogStartDistance = (float(__fastcall*)(const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_FOG_START);
    o_RenderSettings_get_fogEndDistance = (float(__fastcall*)(const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_FOG_END);
    o_RenderSettings_get_fogColor_Injected = (void(__fastcall*)(Color*, const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_FOG_COLOR);
    o_RenderSettings_get_fogDensity = (float(__fastcall*)(const Il2CppMethod*))(base + OFFSET_RENDERSETTINGS_GET_FOG_DENSITY);
    if (g_il2cpp.resolve_icall) {
        o_RenderSettings_set_fog = (void(__fastcall*)(bool))g_il2cpp.resolve_icall("UnityEngine.RenderSettings::set_fog(System.Boolean)");
        o_RenderSettings_set_fogStartDistance = (void(__fastcall*)(float))g_il2cpp.resolve_icall("UnityEngine.RenderSettings::set_fogStartDistance(System.Single)");
        o_RenderSettings_set_fogEndDistance = (void(__fastcall*)(float))g_il2cpp.resolve_icall("UnityEngine.RenderSettings::set_fogEndDistance(System.Single)");
        o_RenderSettings_set_fogColor_Injected = (void(__fastcall*)(Color*))g_il2cpp.resolve_icall("UnityEngine.RenderSettings::set_fogColor_Injected(UnityEngine.Color&)");
        o_RenderSettings_set_fogDensity = (void(__fastcall*)(float))g_il2cpp.resolve_icall("UnityEngine.RenderSettings::set_fogDensity(System.Single)");
    }
    strcpy_s(fogStatus, FogSetterApiReady() ? "Setter API ready" : "Unity fog setter API unavailable");

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
    o_Object_IsAlive = (bool(__fastcall*)(uintptr_t, const Il2CppMethod*))(base + OFFSET_OBJECT_OP_IMPLICIT);
    o_Physics_RaycastHit = (bool(__fastcall*)(Vector3, Vector3, RaycastHitNative*, float, int, int, const Il2CppMethod*))(base + OFFSET_PHYSICS_RAYCAST_HIT);
    o_Physics_RaycastAll = (Il2CppArray*(__fastcall*)(Vector3, Vector3, float, int, int, const Il2CppMethod*))(base + OFFSET_PHYSICS_RAYCAST_ALL);
    o_RaycastHit_get_collider = (uintptr_t(__fastcall*)(RaycastHitNative*, const Il2CppMethod*))(base + OFFSET_RAYCASTHIT_GET_COLLIDER);
    o_Component_GetInParent = (uintptr_t(__fastcall*)(uintptr_t, uintptr_t, bool, const Il2CppMethod*))(base + OFFSET_COMPONENT_GET_IN_PARENT);
    o_Component_get_tag = (Il2CppString*(__fastcall*)(uintptr_t, const Il2CppMethod*))(base + OFFSET_COMPONENT_GET_TAG);
    o_SurfaceType_FromTag = (int(__fastcall*)(Il2CppString*, const Il2CppMethod*))(base + OFFSET_SURFACE_TYPE_FROM_TAG);
    o_Component_GetInChildren = (uintptr_t(__fastcall*)(uintptr_t, uintptr_t, bool, const Il2CppMethod*))(base + OFFSET_COMPONENT_GET_IN_CHILDREN);
    o_Transform_get_parent = (uintptr_t(__fastcall*)(uintptr_t, const Il2CppMethod*))(base + OFFSET_TRANSFORM_GET_PARENT);
    o_GameObject_get_activeInHierarchy = (bool(__fastcall*)(uintptr_t))(base + OFFSET_GAMEOBJECT_GET_ACTIVEINHIERARCHY);
    o_GameChatHud_SendMessage = (void(__fastcall*)(uintptr_t, Il2CppString*, const Il2CppMethod*))(base + OFFSET_GAMECHATHUD_SEND_MESSAGE);
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
    const MH_STATUS hitLogCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_HITMARKERVIEW_LOCAL_HIT), hk_HitMarkerView_LocalHit,
        (LPVOID*)&o_HitMarkerView_LocalHit);
    const MH_STATUS hitLogEnableStatus = hitLogCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_HITMARKERVIEW_LOCAL_HIT)) : hitLogCreateStatus;
    if (hitLogCreateStatus != MH_OK || hitLogEnableStatus != MH_OK)
        hitLogEnabled = false;
    const MH_STATUS antiAimCommandCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_PLAYERCONTROLLER_COMMAND),
        hk_PlayerController_Command,
        (LPVOID*)&o_PlayerController_Command);
    const MH_STATUS antiAimCommandEnableStatus =
        antiAimCommandCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_PLAYERCONTROLLER_COMMAND)) :
        antiAimCommandCreateStatus;
    const MH_STATUS antiAimInputCreateStatus = MH_CreateHook(
        (LPVOID)(base + OFFSET_KEYBOARDCONTROL_BUILD_COMMAND),
        hk_KeyboardControl_BuildCommand,
        (LPVOID*)&o_KeyboardControl_BuildCommand);
    const MH_STATUS antiAimInputEnableStatus =
        antiAimInputCreateStatus == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_KEYBOARDCONTROL_BUILD_COMMAND)) :
        antiAimInputCreateStatus;
    silentAntiAimHookReady = antiAimCommandCreateStatus == MH_OK &&
        antiAimCommandEnableStatus == MH_OK &&
        antiAimInputCreateStatus == MH_OK &&
        antiAimInputEnableStatus == MH_OK;

    LINDY_LOG("[anti-aim] input=%d/%d; command=%d/%d; ready=%d",
        (int)antiAimInputCreateStatus, (int)antiAimInputEnableStatus,
        (int)antiAimCommandCreateStatus, (int)antiAimCommandEnableStatus,
        silentAntiAimHookReady ? 1 : 0);

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
    MH_CreateHook((LPVOID)(base + OFFSET_GUNCONTROLLER_COMMAND), hk_GunController_Command, (LPVOID*)&o_GunController_Command);
    MH_EnableHook((LPVOID)(base + OFFSET_GUNCONTROLLER_COMMAND));
    MH_CreateHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE), hk_GunController_Fire, (LPVOID*)&o_GunController_Fire);
    MH_EnableHook((LPVOID)(base + OFFSET_GUNCONTROLLER_FIRE));
    const MH_STATUS hitCasterCreate = MH_CreateHook(
        (LPVOID)(base + OFFSET_HITCASTER_CAST), hk_HitCaster_Cast,
        (LPVOID*)&o_HitCaster_Cast);
    const MH_STATUS hitCasterEnable = hitCasterCreate == MH_OK ?
        MH_EnableHook((LPVOID)(base + OFFSET_HITCASTER_CAST)) : hitCasterCreate;
    sprintf_s(aimbotStatus, "dump1 HitCaster hook=%d/%d",
        (int)hitCasterCreate, (int)hitCasterEnable);

    MH_CreateHook((LPVOID)(base + OFFSET_RAGDOLL_ACTIVATE), hk_RagdollActivate, (LPVOID*)&o_RagdollActivate);
    MH_EnableHook((LPVOID)(base + OFFSET_RAGDOLL_ACTIVATE));
    MH_CreateHook((LPVOID)(base + OFFSET_RAGDOLL_MANAGER_RELEASE), hk_RagdollManagerRelease, (LPVOID*)&o_RagdollManagerRelease);
    MH_EnableHook((LPVOID)(base + OFFSET_RAGDOLL_MANAGER_RELEASE));

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
