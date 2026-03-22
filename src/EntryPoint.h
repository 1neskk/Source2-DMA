#pragma once
#include "imgui.h"

extern Application *CreateApplication(int argc, char **argv);
inline bool g_bRunning = true;

inline int Main(int argc, char **argv)
{
    while (g_bRunning)
    {
        Application *app = CreateApplication(argc, argv);
        app->Run();
        delete app;
    }
    return 0;
}

INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return Main(__argc, __argv);
}

namespace Style
{
inline void Theme()
{
    auto &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // ── Base palette ──────────────────────────────────────────────
    const ImVec4 bg        = ImVec4(0.102f, 0.102f, 0.133f, 1.00f); // #1A1A22
    const ImVec4 bgChild   = ImVec4(0.118f, 0.118f, 0.157f, 1.00f); // #1E1E28
    const ImVec4 surface   = ImVec4(0.145f, 0.145f, 0.180f, 1.00f); // #25252E
    const ImVec4 border    = ImVec4(0.200f, 0.200f, 0.243f, 0.50f); // subtle border
    const ImVec4 accent    = ImVec4(0.486f, 0.227f, 0.929f, 1.00f); // #7C3AED  Violet-600
    const ImVec4 accentDim = ImVec4(0.486f, 0.227f, 0.929f, 0.70f);
    const ImVec4 accentLo  = ImVec4(0.486f, 0.227f, 0.929f, 0.35f);
    const ImVec4 text      = ImVec4(0.784f, 0.784f, 0.816f, 1.00f); // #C8C8D0
    const ImVec4 textDim   = ImVec4(0.475f, 0.475f, 0.525f, 1.00f);
    const ImVec4 hoverBg   = ImVec4(0.200f, 0.200f, 0.243f, 1.00f);

    // ── Window ────────────────────────────────────────────────────
    colors[ImGuiCol_WindowBg]             = bg;
    colors[ImGuiCol_ChildBg]              = bgChild;
    colors[ImGuiCol_PopupBg]              = ImVec4(0.125f, 0.125f, 0.157f, 0.96f);
    colors[ImGuiCol_Border]               = border;
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // ── Text ──────────────────────────────────────────────────────
    colors[ImGuiCol_Text]                 = text;
    colors[ImGuiCol_TextDisabled]         = textDim;

    // ── Title bar ─────────────────────────────────────────────────
    colors[ImGuiCol_TitleBg]              = ImVec4(0.090f, 0.090f, 0.114f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.110f, 0.110f, 0.140f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.090f, 0.090f, 0.114f, 0.75f);

    // ── Frame (checkboxes, combos, inputs) ───────────────────────
    colors[ImGuiCol_FrameBg]              = surface;
    colors[ImGuiCol_FrameBgHovered]       = hoverBg;
    colors[ImGuiCol_FrameBgActive]        = accentLo;

    // ── Tabs ──────────────────────────────────────────────────────
    colors[ImGuiCol_Tab]                  = surface;
    colors[ImGuiCol_TabHovered]           = accentDim;
    colors[ImGuiCol_TabActive]            = accent;
    colors[ImGuiCol_TabUnfocused]         = surface;
    colors[ImGuiCol_TabUnfocusedActive]   = accentLo;

    // ── Buttons ───────────────────────────────────────────────────
    colors[ImGuiCol_Button]               = surface;
    colors[ImGuiCol_ButtonHovered]        = accentDim;
    colors[ImGuiCol_ButtonActive]         = accent;

    // ── Headers (collapsing headers, tree nodes) ──────────────────
    colors[ImGuiCol_Header]               = surface;
    colors[ImGuiCol_HeaderHovered]        = accentLo;
    colors[ImGuiCol_HeaderActive]         = accentDim;

    // ── Separator ─────────────────────────────────────────────────
    colors[ImGuiCol_Separator]            = border;
    colors[ImGuiCol_SeparatorHovered]     = accentDim;
    colors[ImGuiCol_SeparatorActive]      = accent;

    // ── Scrollbar ─────────────────────────────────────────────────
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.102f, 0.102f, 0.133f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.220f, 0.220f, 0.267f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = accentDim;
    colors[ImGuiCol_ScrollbarGrabActive]  = accent;

    // ── Slider ────────────────────────────────────────────────────
    colors[ImGuiCol_SliderGrab]           = accentDim;
    colors[ImGuiCol_SliderGrabActive]     = accent;

    // ── Check mark ────────────────────────────────────────────────
    colors[ImGuiCol_CheckMark]            = accent;

    // ── Resize grip ───────────────────────────────────────────────
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered]    = accentDim;
    colors[ImGuiCol_ResizeGripActive]     = accent;

    // ── Rounding & layout ─────────────────────────────────────────
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding       = 4.0f;

    style.WindowPadding     = ImVec2(14.0f, 14.0f);
    style.FramePadding      = ImVec2(10.0f, 6.0f);
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;

    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);
    style.AntiAliasedLines  = true;
    style.AntiAliasedFill   = true;
}
} // namespace Style