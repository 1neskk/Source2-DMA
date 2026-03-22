#include "Application.h"
#include "Cheat.h"
#include "Drawing.h"
#include "EntryPoint.h"

class Source2 final : public Layer
{
  public:
    void OnUpdate(const float ts) override
    {
        if (m_cheat.IsAttached())
        {
            m_cheat.OnUpdate();
        }

        if (GetAsyncKeyState(VK_END) & 1)
        {
            Application::Close();
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            if (Drawing::IsActive())
                Drawing::SetActive(false);
            else
                Drawing::SetActive(true);
        }
    }

    void OnImGuiRender() override
    {
        {
            char fpsBuffer[16] = {};
            std::snprintf(fpsBuffer, sizeof(fpsBuffer), "FPS: %.0f", ImGui::GetIO().Framerate);
            ImGui::GetBackgroundDrawList()->AddText(
                ImVec2(ImGui::GetIO().DisplaySize.x - 80.0f, 10.0f),
                IM_COL32(0, 255, 0, 255), fpsBuffer);
        }

        if (Drawing::IsActive())
        {
            Style::Theme();

            if (m_cheat.IsAttached())
                RenderMainMenu();
            else
                RenderLoader();
        }

        if (m_cheat.IsAttached() && m_cheat.ToggleEsp())
            m_cheat.RenderEsp(m_cheat.GetEntities());
    }

  private:
    void RenderLoader()
    {
        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

        ImGui::SetNextWindowSize(ImVec2(340, 220), ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.95f);

        ImGui::Begin("Nesk DMA", nullptr, flags);

        if (g_FontBold)
            ImGui::PushFont(g_FontBold);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Source 2 DMA").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.486f, 0.227f, 0.929f, 1.0f), "Source 2 DMA");
        if (g_FontBold)
            ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Target Process");
        const char *games[] = {"cs2.exe", "deadlock.exe"};
        static int current_item = 0;

        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##gameCombo", &current_item, games, IM_ARRAYSIZE(games)))
            m_cheat.m_gameName = static_cast<std::string>(games[current_item]);
        else if (m_cheat.m_gameName.empty())
            m_cheat.m_gameName = static_cast<std::string>(games[0]);

        ImGui::Spacing();
        ImGui::Spacing();

        const float btnWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        if (ImGui::Button("Attach", ImVec2(btnWidth, 32)))
            m_cheat.Attach(m_cheat.m_gameName);

        ImGui::SameLine();

        if (ImGui::Button("Exit", ImVec2(btnWidth, 32)))
            Application::Close();

        ImGui::End();
    }

    void RenderMainMenu()
    {
        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

        ImGui::SetNextWindowSize(ImVec2(420, 480), ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.95f);

        ImGui::Begin("Nesk DMA", nullptr, flags);

        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "\xE2\x97\x8F"); // green dot ●
        ImGui::SameLine();
        ImGui::Text("%s  |  PID %d", m_cheat.m_gameName.c_str(), m_cheat.GetProcessId());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("##mainTabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("ESP"))
            {
                ImGui::Spacing();

                if (g_FontBold) ImGui::PushFont(g_FontBold);
                ImGui::Checkbox("Enable ESP", &m_cheat.m_bEsp);
                if (g_FontBold) ImGui::PopFont();

                if (m_cheat.m_bEsp)
                {
                    ImGui::Spacing();
                    ImGui::Indent(12.0f);

                    ImGui::Checkbox("Box",        &m_cheat.m_bBox);
                    ImGui::Checkbox("Bones",      &m_cheat.m_bBones);
                    ImGui::Checkbox("Name",       &m_cheat.m_bName);
                    ImGui::Checkbox("Health Bar",  &m_cheat.m_bHealth);
                    ImGui::Checkbox("Distance",   &m_cheat.m_bDistance);
                    ImGui::Checkbox("Snaplines",  &m_cheat.m_bSnaplines);

                    ImGui::Unindent(12.0f);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Checkbox("Show Team ESP", &m_cheat.m_bTeamEsp);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Misc"))
            {
                ImGui::Spacing();

                if (g_FontBold) ImGui::PushFont(g_FontBold);
                ImGui::Text("Movement");
                if (g_FontBold) ImGui::PopFont();

                ImGui::Spacing();
                ImGui::Indent(12.0f);
                ImGui::Checkbox("Bunny Hop", &m_cheat.m_bBunnyHop);
                ImGui::Unindent(12.0f);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (g_FontBold) ImGui::PushFont(g_FontBold);
                ImGui::Text("Aim Assist");
                if (g_FontBold) ImGui::PopFont();

                ImGui::Spacing();
                ImGui::Indent(12.0f);
                ImGui::Checkbox("Aimbot", &m_cheat.m_bAimbot);
                ImGui::Unindent(12.0f);

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::Spacing();

                if (g_FontBold) ImGui::PushFont(g_FontBold);
                ImGui::Text("Process Info");
                if (g_FontBold) ImGui::PopFont();

                ImGui::Spacing();
                ImGui::Indent(12.0f);
                ImGui::Text("Game:         %s", m_cheat.m_gameName.c_str());
                ImGui::Text("PID:          %d", m_cheat.GetProcessId());
                ImGui::Text("Base Address: 0x%llX", static_cast<unsigned long long>(m_cheat.GetBaseAddress()));
                ImGui::Unindent(12.0f);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Detach & Exit", ImVec2(-1, 32)))
                {
                    m_cheat.Detach();
                    Application::Close();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

  private:
    Cheat m_cheat{};
};

Application *CreateApplication(int argc, char **argv)
{
    Specs spec;
    spec.title = "Source2 DMA";

    auto app = new Application(spec);
    app->PushLayer<Source2>();

    return app;
}
