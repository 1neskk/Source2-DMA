#include "Cheat.h"

#include <array>
#include <numbers>
#include <set>

#include "DeadlockOffsets.h"
#include "DMALibrary/Memory/Memory.h"
#include "imgui.h"

Cheat::~Cheat()
{
    m_running = false;
    if (m_workerThread.joinable())
        m_workerThread.join();

    m_dwProcessId = 0;
    m_baseAddress = 0;
    m_localPlayerData.EntityPawn = 0;
    m_entityList = 0;

    mem.CloseScatterHandle(m_scatterHandle);

#ifdef _DEBUG
    FreeConsole();
#endif
}

auto Cheat::Attach(const std::string &processName) -> void
{
#ifdef _DEBUG
    AllocConsole();
    freopen_s(reinterpret_cast<FILE **>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE **>(stderr), "CONOUT$", "w", stderr);

    if (!mem.Init(processName, true, true))
    {
        std::cerr << "[-] Failed to initialize DMA\n";
        return;
    }
#endif

#ifndef _DEBUG
    if (!mem.Init(processName, true, false))
    {
        std::cerr << "[-] Failed to initialize DMA\n";
        return;
    }
#endif

    std::cout << "[+] DMA successfully initialized\n";

    m_baseAddress = mem.GetBaseDaddy("client.dll");
    m_dwProcessId = mem.GetPidFromName(processName);
#ifdef _DEBUG
    std::cout << "[+] Found PID for " << processName << ": " << m_dwProcessId << "\n";
#endif
    m_scatterHandle = mem.CreateScatterHandle(static_cast<int>(m_dwProcessId));

    if (m_gameName == "deadlock.exe")
    {
        const size_t clientSize = mem.GetBaseSize("client.dll");
        const bool scanned = DeadlockOffsetGenerator::Scan(
            m_baseAddress, clientSize, m_dwProcessId, m_dlOffsets);
        if (!scanned)
        {
            std::cout << "[Deadlock] Falling back to hardcoded offsets.\n";
            m_dlOffsets.dwEntityList            = 0x3176250;
            m_dlOffsets.dwLocalPlayerController = 0x2E76FE8;
            m_dlOffsets.dwViewMatrix            = 0x3728010;
        }
    }

#ifdef _DEBUG
    std::cout << "[Deadlock] Offsets in use:\n"
              << "  dwEntityList:            0x" << std::hex << m_dlOffsets.dwEntityList << std::dec << "\n"
              << "  dwLocalPlayerController: 0x" << std::hex << m_dlOffsets.dwLocalPlayerController - m_baseAddress << std::dec << "\n"
              << "  dwViewMatrix:            0x" << std::hex << m_dlOffsets.dwViewMatrix << std::dec << "\n";
#endif

    m_width = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
    m_height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

    m_running = true;
    m_workerThread = std::thread(&Cheat::WorkerLoop, this);
}

void Cheat::Detach()
{
    m_running = false;
    if (m_workerThread.joinable())
        m_workerThread.join();
    this->~Cheat();
}

bool Cheat::IsAttached() const
{
    return m_dwProcessId && m_baseAddress;
}

void Cheat::WorkerLoop()
{
    // Create a dedicated scatter handle for this thread
    const auto workerScatterHandle = mem.CreateScatterHandle(static_cast<int>(m_dwProcessId));

    while (m_running)
    {
        // Read local player data
        if (!m_localPlayerData.EntityPawn)
        {
            if (m_gameName == "cs2.exe")
                mem.AddScatterReadRequest(workerScatterHandle,
                                          m_baseAddress + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn,
                                          &m_localPlayerData.EntityPawn, sizeof(uintptr_t));
            else
                mem.AddScatterReadRequest(workerScatterHandle,
                                          m_dlOffsets.dwLocalPlayerController,
                                          &m_localPlayerData.EntityPawn, sizeof(uintptr_t));
        }

        if (!m_entityList)
        {
            if (m_gameName == "cs2.exe")
                mem.AddScatterReadRequest(workerScatterHandle,
                                          m_baseAddress + cs2_dumper::offsets::client_dll::dwEntityList, &m_entityList,
                                          sizeof(m_entityList));
            else
                mem.AddScatterReadRequest(workerScatterHandle,
                                          m_dlOffsets.dwEntityList,
                                          &m_entityList, sizeof(m_entityList));
        }

        if (!m_localPlayerData.EntityPawn || !m_entityList)
        {
            mem.ExecuteReadScatter(workerScatterHandle, m_dwProcessId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

#ifdef _DEBUG
        static bool loggedOnce = false;
        if (!loggedOnce)
        {
            std::cout << "[DBG] entityPawn: 0x" << std::hex << m_localPlayerData.EntityPawn << " | entityList: 0x"
                      << m_entityList << std::dec << "\n";
            if (m_localPlayerData.EntityPawn && m_entityList)
                loggedOnce = true;
        }
#endif

        if (m_gameName == "cs2.exe")
            mem.AddScatterReadRequest(workerScatterHandle,
                                    m_baseAddress + cs2_dumper::offsets::client_dll::dwViewMatrix,
                                    &m_viewMatrix, sizeof(m_viewMatrix));
        else
            mem.AddScatterReadRequest(workerScatterHandle,
                                      m_dlOffsets.dwViewMatrix,
                                      &m_viewMatrix, sizeof(m_viewMatrix));


        if (m_gameName == "cs2.exe")
            mem.AddScatterReadRequest(workerScatterHandle,
                                      m_localPlayerData.EntityPawn +
                                      cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum,
                                      &m_localPlayerData.Team, sizeof(uint8_t));
        else
            mem.AddScatterReadRequest(workerScatterHandle,
                                      m_localPlayerData.EntityPawn + m_dlOffsets.m_iTeamNum,
                                      &m_localPlayerData.Team, sizeof(uint8_t));

        if (m_gameName == "cs2.exe")
            mem.AddScatterReadRequest(workerScatterHandle,
                                  m_localPlayerData.EntityPawn +
                                  cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin,
                                  &m_localPlayerData.Origin, sizeof(m_localPlayerData.Origin));
        else
            mem.AddScatterReadRequest(workerScatterHandle,
                                      m_localPlayerData.EntityPawn + m_dlOffsets.m_vOldOrigin,
                                      &m_localPlayerData.Origin, sizeof(m_localPlayerData.Origin));

        mem.ExecuteReadScatter(workerScatterHandle, m_dwProcessId);

        GetEntityData();

        // Swap the back buffer into the front buffer under lock
        {
            std::scoped_lock lock(m_dataMutex);
            m_entities.swap(m_backBuffer);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    mem.CloseScatterHandle(workerScatterHandle);
}

void Cheat::OnUpdate()
{
    if (GetAsyncKeyState(VK_END) & 1)
        Detach();
}

void Cheat::RenderEsp(const std::vector<EntityData> &entities) const
{
#ifdef _DEBUG
    if (entities.empty())
    {
        std::cerr << "[-] No entities found\n";
        return;
    }
    static std::set<uintptr_t> printedEntities;
    if (printedEntities.empty())
    {
        for (const auto &entity : entities)
        {
            std::cout << "[+] Found Entity: " << entity.Name << " | Pawn: 0x" << std::hex << entity.EntityPawn
                      << std::dec << " | Team: " << entity.Team << " | Health: " << entity.Health << "\n";
            printedEntities.insert(entity.EntityPawn);
        }
    }
#endif

    constexpr int HEAD_BONE = 6;

    for (const auto &entity : entities)
    {
        if (!m_bTeamEsp && entity.Team == m_localPlayerData.Team)
            continue;

        const glm::vec3 head = entity.Bones.valid
                                   ? entity.Bones[HEAD_BONE].location
                                   : glm::vec3{entity.Origin.x, entity.Origin.y, entity.Origin.z + 72.0f};

        glm::vec3 screenHead = head;
        screenHead.z += 10.0f;

        glm::vec3 screenOrigin = entity.Origin;
        screenOrigin.z += 10.0f;

        if (WorldToScreen(head, screenHead, m_viewMatrix) && WorldToScreen(entity.Origin, screenOrigin, m_viewMatrix))
        {
            const auto distance = glm::distance(m_localPlayerData.Origin, entity.Origin) / 10.0f;
            const auto healthPercentage = static_cast<float>(entity.Health) / 100.f;

            const auto healthColor = IM_COL32(255 * (1 - healthPercentage), 255 * healthPercentage, 0, 255);
            constexpr auto boxColor = ImColor(255, 0, 0, 255);
            constexpr auto skeletonColor = ImColor(255, 255, 255, 255);

            const auto width = (screenOrigin.y - screenHead.y) / 2;
            const auto height = screenOrigin.y - screenHead.y;

            if (m_bBox)
            {
                ImGui::GetBackgroundDrawList()->AddRect(ImVec2(screenHead.x - width / 2, screenHead.y - 8.0f),
                                                        ImVec2(screenHead.x + width / 2, screenHead.y + height), boxColor,
                                                        0.f, 0, 1.2f);
            }

            if (m_bHealth)
            {
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(screenHead.x - width / 2, screenOrigin.y),
                    ImVec2(screenHead.x - width / 2 + 5, screenOrigin.y - height * healthPercentage), ImColor(healthColor),
                    0.f, 0);
                ImGui::GetBackgroundDrawList()->AddRect(ImVec2(screenHead.x - width / 2, screenHead.y - 6.0f),
                                                        ImVec2(screenHead.x - width / 2 + 5, screenHead.y + height),
                                                        ImColor(0, 0, 0, 255), 0.f, 0, 1.2f);
            }

            if (m_bBones && entity.Bones.valid)
            {
                for (const auto &[b1, b2] : SKELETON_BONES)
                {
                    if (entity.Bones[b1].location != glm::vec3{0.0f, 0.0f, 0.0f} &&
                        entity.Bones[b2].location != glm::vec3{0.0f, 0.0f, 0.0f})
                    {
                        glm::vec3 screenBone1, screenBone2;
                        if (WorldToScreen(entity.Bones[b1].location, screenBone1, m_viewMatrix) &&
                            WorldToScreen(entity.Bones[b2].location, screenBone2, m_viewMatrix))
                        {
                            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screenBone1.x, screenBone1.y),
                                                                    ImVec2(screenBone2.x, screenBone2.y), skeletonColor,
                                                                    1.2f);
                        }
                    }
                }
            }

            if (m_bName)
            {
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenHead.x - width / 2, screenHead.y - 23.0f),
                                                        ImColor(255, 255, 255, 255), entity.Name.c_str());
            }

            if (m_bDistance && distance < 1000.f)
            {
                const auto distanceText = std::format("{:.2f}m", distance);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenHead.x - width / 2, screenHead.y + height),
                                                        ImColor(255, 255, 255, 255), distanceText.c_str());
            }

            if (m_bSnaplines)
            {
                ImGui::GetBackgroundDrawList()->AddLine(
                    ImVec2(m_width / 2.0f, m_height),
                    ImVec2(screenOrigin.x, screenOrigin.y),
                    IM_COL32(124, 58, 237, 180), 1.0f);
            }
        }
    }
}

void Cheat::GetEntityData()
{
    // Zero-init the scratch pipeline data
    m_pipeline = {};
    m_backBuffer.clear();

#ifdef _DEBUG
    static bool dbgPipeline = false;
#endif

    // Create a scatter handle dedicated to entity data reads within this function
    const auto scatterHandle = mem.CreateScatterHandle(m_dwProcessId);

    // Stage 1: Read entity list entries
    for (int i = 0; i < 32; i++)
    {
        mem.AddScatterReadRequest(scatterHandle, m_entityList + (0x8 * (i & 0x7FFF) >> 9) + 0x10,
                                  &m_pipeline.ListEntry1[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        int cnt = 0;
        for (int i = 0; i < 32; i++)
            if (m_pipeline.ListEntry1[i])
                cnt++;
        std::cout << "[DBG] Stage 1 - listEntry1 valid: " << cnt << "/32\n";
        if (cnt > 0)
            std::cout << "[DBG]   listEntry1[0]=0x" << std::hex << m_pipeline.ListEntry1[0] << std::dec << "\n";
    }
#endif

    // Stage 2: Read entity controllers
    for (int i = 0; i < 32; i++)
    {
        if (m_pipeline.ListEntry1[i])
        {
            mem.AddScatterReadRequest(scatterHandle, m_pipeline.ListEntry1[i] + 0x70 * (i & 0x1FF),
                                      &m_pipeline.EntityController[i], sizeof(uintptr_t));
        }
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Stage 2 - entityController details:\n";
        for (int i = 0; i < 32; i++)
        {
            if (m_pipeline.EntityController[i])
                std::cout << "[DBG]   [" << i << "] ctrl=0x" << std::hex << m_pipeline.EntityController[i] << std::dec
                          << "\n";
        }
    }
#endif

    // Stage 3: Read player pawn handles from controllers
    for (int i = 0; i < 32; i++)
    {
        if (m_pipeline.EntityController[i])
        {
            if (m_gameName == "cs2.exe")
                mem.AddScatterReadRequest(scatterHandle,
                                          m_pipeline.EntityController[i] +
                                              cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn,
                                          &m_pipeline.EntityControllerPawn[i], sizeof(uint32_t));
            else
                mem.AddScatterReadRequest(scatterHandle,
                                          m_pipeline.EntityController[i] +
                                          m_dlOffsets.m_hPlayerPawn,
                                          &m_pipeline.EntityControllerPawn[i], sizeof(uint32_t));
        }
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Stage 3 - m_hPawn (offset 0x" << std::hex
                  << cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn << std::dec << ") details:\n";
        for (int i = 0; i < 32; i++)
        {
            if (m_pipeline.EntityController[i])
            {
                std::cout << "[DBG]   [" << i << "] ctrl=0x" << std::hex << m_pipeline.EntityController[i]
                          << " -> hPawn=0x" << m_pipeline.EntityControllerPawn[i] << std::dec << "\n";
            }
        }
    }
#endif

    // Stage 4: Resolve pawn entity list entries
    for (int i = 0; i < 32; i++)
    {
        if (m_pipeline.EntityControllerPawn[i])
        {
            mem.AddScatterReadRequest(scatterHandle,
                                      m_entityList + 0x8 * ((m_pipeline.EntityControllerPawn[i] & 0x7FFF) >> 9) + 0x10,
                                      &m_pipeline.ListEntry2[i], sizeof(uintptr_t));
        }
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Stage 4 - listEntry2 details:\n";
        for (int i = 0; i < 32; i++)
        {
            if (m_pipeline.EntityControllerPawn[i])
                std::cout << "[DBG]   [" << i << "] hPawn=0x" << std::hex << m_pipeline.EntityControllerPawn[i]
                          << " -> listEntry2=0x" << m_pipeline.ListEntry2[i] << std::dec << "\n";
        }
    }
#endif

    // Stage 5: Read entity pawn pointers
    for (int i = 0; i < 32; i++)
    {
        if (m_pipeline.ListEntry2[i])
        {
            mem.AddScatterReadRequest(scatterHandle,
                                      m_pipeline.ListEntry2[i] + 0x70 * (m_pipeline.EntityControllerPawn[i] & 0x1FF),
                                      &m_pipeline.EntityPawn[i], sizeof(uintptr_t));
        }
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Stage 5 - entityPawn details:\n";
        for (int i = 0; i < 32; i++)
        {
            if (m_pipeline.EntityPawn[i])
                std::cout << "[DBG]   [" << i << "] pawn=0x" << std::hex << m_pipeline.EntityPawn[i] << std::dec
                          << (m_pipeline.EntityPawn[i] == m_localPlayerData.EntityPawn ? " (LOCAL)" : "") << "\n";
        }
    }
#endif

    // Stage 6: Read entity properties (team, health, origin, name, gameSceneNode)
    for (int i = 0; i < 32; i++)
    {
        if (!m_pipeline.EntityPawn[i] || m_pipeline.EntityPawn[i] == m_localPlayerData.EntityPawn)
            continue;

        if (m_gameName == "cs2.exe")
        {
            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum,
                &m_pipeline.Team[i], sizeof(uint8_t));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth,
                &m_pipeline.Health[i], sizeof(int));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin,
                &m_pipeline.Origin[i], sizeof(glm::vec3));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityController[i] + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName,
                &m_pipeline.NameBuffer[i], sizeof(m_pipeline.NameBuffer[i]));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode,
                &m_pipeline.GameSceneNode[i], sizeof(uintptr_t));
        }
        else
        {
            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + m_dlOffsets.m_iTeamNum,
                &m_pipeline.Team[i], sizeof(uint8_t));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + m_dlOffsets.m_iHealth,
                &m_pipeline.Health[i], sizeof(int));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + m_dlOffsets.m_vOldOrigin,
                &m_pipeline.Origin[i], sizeof(glm::vec3));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityController[i] + m_dlOffsets.m_iszPlayerName,
                &m_pipeline.NameBuffer[i], sizeof(m_pipeline.NameBuffer[i]));

            mem.AddScatterReadRequest(scatterHandle,
                m_pipeline.EntityPawn[i] + m_dlOffsets.m_pGameSceneNode,
                &m_pipeline.GameSceneNode[i], sizeof(uintptr_t));
        }
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

    // Stage 7: Read bone array pointer from CSkeletonInstance::m_modelState +
    // BONE_ARRAY_OFFSET
    for (int i = 0; i < 32; i++)
    {
        if (!m_pipeline.GameSceneNode[i] || !m_pipeline.EntityPawn[i] ||
            m_pipeline.EntityPawn[i] == m_localPlayerData.EntityPawn)
            continue;

        mem.AddScatterReadRequest(scatterHandle,
                                  m_pipeline.GameSceneNode[i] +
                                      cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState +
                                      BONE_ARRAY_OFFSET,
                                  &m_pipeline.BoneArray[i], sizeof(uintptr_t));
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

    // Stage 8: Read actual bone data from bone array
    for (int i = 0; i < 32; i++)
    {
        if (!m_pipeline.BoneArray[i])
            continue;

        mem.AddScatterReadRequest(scatterHandle, m_pipeline.BoneArray[i], &m_pipeline.BoneMatrices[i].bones,
                                  sizeof(BoneData) * MAX_BONES);
        m_pipeline.BoneMatrices[i].valid = true;
    }
    mem.ExecuteReadScatter(scatterHandle, m_dwProcessId);

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Stage 6 - entity data details:\n";
        for (int i = 0; i < 32; i++)
        {
            if (!m_pipeline.EntityPawn[i] || m_pipeline.EntityPawn[i] == m_localPlayerData.EntityPawn)
                continue;
            std::cout << "[DBG]   [" << i << "] pawn=0x" << std::hex << m_pipeline.EntityPawn[i] << std::dec
                      << " health=" << m_pipeline.Health[i] << " team=" << static_cast<int>(m_pipeline.Team[i])
                      << " origin=(" << m_pipeline.Origin[i].x << "," << m_pipeline.Origin[i].y << ","
                      << m_pipeline.Origin[i].z << ")"
                      << " name=\""
                      << std::string(m_pipeline.NameBuffer[i].data(), strnlen(m_pipeline.NameBuffer[i].data(), 32))
                      << "\"\n";
        }
    }
#endif

    // Build the entity list
    for (int i = 0; i < 32; i++)
    {
        if (!m_pipeline.EntityPawn[i] || m_pipeline.EntityPawn[i] == m_localPlayerData.EntityPawn)
            continue;

        if (m_pipeline.Health[i] <= 0 || m_pipeline.Health[i] > 100)
        {
            continue;
        }

        if (!m_bTeamEsp && m_pipeline.Team[i] == m_localPlayerData.Team)
        {
            continue;
        }

        m_backBuffer.emplace_back(
            EntityData{m_pipeline.EntityPawn[i], m_pipeline.EntityController[i], m_pipeline.Team[i],
                       m_pipeline.Health[i], m_pipeline.Origin[i],
                       std::string(m_pipeline.NameBuffer[i].data(),
                       strnlen(m_pipeline.NameBuffer[i].data(), m_pipeline.NameBuffer[i].size())),
                       m_pipeline.BoneMatrices[i]});
    }

#ifdef _DEBUG
    if (!dbgPipeline)
    {
        std::cout << "[DBG] Final entity count: " << m_backBuffer.size() << "\n";
        dbgPipeline = true;
    }
#endif

    mem.CloseScatterHandle(scatterHandle);
}

bool Cheat::WorldToScreen(const glm::vec3 &pos, glm::vec3 &screen, const glm::mat4 &viewMatrix) const
{
    const auto w = viewMatrix[3].x * pos.x + viewMatrix[3].y * pos.y + viewMatrix[3].z * pos.z + viewMatrix[3].w;
    if (w < 0.001f)
        return false;
    const auto x = viewMatrix[0].x * pos.x + viewMatrix[0].y * pos.y + viewMatrix[0].z * pos.z + viewMatrix[0].w;
    const auto y = viewMatrix[1].x * pos.x + viewMatrix[1].y * pos.y + viewMatrix[1].z * pos.z + viewMatrix[1].w;
    screen.x = (m_width / 2.0f) * (1.0f + x / w);
    screen.y = (m_height / 2.0f) * (1.0f - y / w);
    screen.z = w;
    return true;
}
