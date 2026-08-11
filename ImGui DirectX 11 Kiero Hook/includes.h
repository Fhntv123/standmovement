#pragma once 

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define WIN32_LEAN_AND_MEAN

// Windows
#include <Windows.h>
#include <winuser.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <urlmon.h>
#include <wininet.h>
#include <wincodec.h>
#include <comdef.h>

// C++ Standard
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>

// DirectX 11
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// Kiero
#include "kiero.h"

// MinHook
#include "kiero/minhook/include/MinHook.h"

// Link libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "windowscodecs.lib")

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API __declspec(dllimport)
#endif

// Function declarations
typedef HRESULT(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
extern Present oPresent;
extern HWND window;
extern WNDPROC oWndProc;
extern ID3D11Device* pDevice;
extern ID3D11DeviceContext* pContext;
extern ID3D11RenderTargetView* mainRenderTargetView;
extern bool init;
extern bool keyValidated;
extern bool unloadRequested;