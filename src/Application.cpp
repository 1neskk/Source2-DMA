#include "Application.h"

#include <dwmapi.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Drawing.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "imgui/Roboto-Regular.embed"

extern bool g_bRunning;

ImFont *g_FontRegular = nullptr;
ImFont *g_FontBold = nullptr;

ID3D11Device *Application::s_pd3dDevice = nullptr;
ID3D11DeviceContext *Application::s_pd3dDeviceContext = nullptr;
IDXGISwapChain *Application::s_pSwapChain = nullptr;
ID3D11RenderTargetView *Application::s_mainRenderTargetView = nullptr;

HWND hwnd = nullptr;

static Application *s_instance = nullptr;

bool Application::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 0; // Uncapped
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern SwapEffect needed for tearing
    }

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 1,
                                      D3D11_SDK_VERSION, &sd, &s_pSwapChain, &s_pd3dDevice, &featureLevel,
                                      &s_pd3dDeviceContext) != S_OK)
    {
        return false;
    }

    CreateRenderTarget();
    return true;
}

void Application::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (s_pSwapChain)
    {
        s_pSwapChain->Release();
        s_pSwapChain = nullptr;
    }
    if (s_pd3dDeviceContext)
    {
        s_pd3dDeviceContext->Release();
        s_pd3dDeviceContext = nullptr;
    }
    if (s_pd3dDevice)
    {
        s_pd3dDevice->Release();
        s_pd3dDevice = nullptr;
    }
}

void Application::CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer;
    s_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    s_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &s_mainRenderTargetView);
    pBackBuffer->Release();
}

void Application::CleanupRenderTarget()
{
    if (s_mainRenderTargetView)
    {
        s_mainRenderTargetView->Release();
        s_mainRenderTargetView = nullptr;
    }
}

LRESULT WINAPI Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (s_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            s_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Application::Application(const Specs &specs) : m_specs(specs)
{
    s_instance = this;
    Init();
}

Application::~Application()
{
    Shutdown();
    s_instance = nullptr;
}

Application &Application::Get()
{
    return *s_instance;
}

void Application::Init()
{
    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc;
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"Source 2 DMA";
    wc.lpszMenuName = nullptr;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassExW(&wc);
    hwnd = CreateWindowExW(WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, L"Source 2 DMA",
                           WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr,
                           nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = {-1, 0, 0, 0};
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(s_pd3dDevice, s_pd3dDeviceContext);

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    g_FontRegular = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f, &config);
    if (!g_FontRegular)
        g_FontRegular = io.Fonts->AddFontDefault();
    g_FontBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 17.0f, &config);
    if (!g_FontBold)
        g_FontBold = g_FontRegular;
    io.FontDefault = g_FontRegular;
}

void Application::Shutdown()
{
    for (auto &layer : m_layers)
        layer->OnDetach();

    m_layers.clear();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();

    g_bRunning = false;
}

void Application::Run()
{
    ImVec4 clearColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImGuiIO &io = ImGui::GetIO();

    while (g_bRunning)
    {
        for (auto &layer : m_layers)
            layer->OnUpdate(m_timeStep);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            break;

        s_pd3dDeviceContext->OMSetRenderTargets(1, &s_mainRenderTargetView, nullptr);
        s_pd3dDeviceContext->ClearRenderTargetView(s_mainRenderTargetView, (float *)&clearColor);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        for (auto &layer : m_layers)
            layer->OnImGuiRender();

        if (Drawing::IsActive())
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_POPUP | WS_EX_LAYERED);
        else
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED);

        ImGui::Render();
        s_pd3dDeviceContext->OMSetRenderTargets(1, &s_mainRenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        s_pSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING); // V-Sync disabled, tearing allowed

        const float targetFrameTime = 1.0f / 240.0f;
        float time = GetTime();
        m_frameTime = time - m_lastFrameTime;

        m_timeStep = (((m_frameTime) < (0.0333f)) ? (m_frameTime) : (0.0333f));
        m_lastFrameTime = time;
    }
}

void Application::Close()
{
    g_bRunning = false;
}

float Application::GetTime()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return float(li.QuadPart) / float(freq.QuadPart);
}