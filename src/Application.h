#pragma once

#include <Windows.h>
#include <d3d11.h>

#include <memory>
#include <string>
#include <vector>

#include "imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern ImFont *g_FontRegular;
extern ImFont *g_FontBold;

class Layer
{
  public:
    virtual ~Layer() = default;

    virtual void OnAttach()
    {
    }
    virtual void OnDetach()
    {
    }

    virtual void OnUpdate(float ts)
    {
    }
    virtual void OnImGuiRender()
    {
    }
};

struct Specs
{
    std::string title;
    uint32_t width = 600, height = 600;
};

class Application
{
  public:
    Application(const Specs &specs = Specs());
    ~Application();

    static Application &Get();

    void Run();

    template <typename T> void PushLayer()
    {
        static_assert(std::is_base_of<Layer, T>::value, "T must derive from Layer");
        m_layers.emplace_back(std::make_shared<T>())->OnAttach();
    }

    void PushLayer(const std::shared_ptr<Layer> &layer)
    {
        m_layers.emplace_back(layer);
        layer->OnAttach();
    }

    static void Close();
    float GetTime();

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static bool CreateDeviceD3D(HWND hWnd);
    static void CleanupDeviceD3D();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();

  private:
    void Init();
    void Shutdown();

  private:
    Specs m_specs;
    std::vector<std::shared_ptr<Layer>> m_layers;

    float m_lastFrameTime = 0.0f;
    float m_frameTime = 0.0f;
    float m_timeStep = 0.0f;

    static ID3D11Device *s_pd3dDevice;
    static ID3D11DeviceContext *s_pd3dDeviceContext;
    static IDXGISwapChain *s_pSwapChain;
    static ID3D11RenderTargetView *s_mainRenderTargetView;
};

Application *CreateApplication(int argc, char **argv);