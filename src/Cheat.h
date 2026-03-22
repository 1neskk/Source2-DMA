#pragma once

#include "Bones.h"
#include "DeadlockOffsets.h"
#include "offsets/buttons.hpp"
#include "offsets/client_dll.hpp"
#include "offsets/offsets.hpp"

typedef HANDLE VMMDLL_SCATTER_HANDLE;

struct EntityData
{
    uintptr_t EntityPawn;
    uintptr_t EntityController;
    uint8_t Team;
    int32_t Health;
    glm::vec3 Origin;
    std::string Name;
    BoneMatrix Bones;
};

struct LocalPlayerData
{
    uintptr_t EntityPawn;
    uint8_t Team;
    glm::vec3 Origin;
};

struct EntityPipelineData
{
    std::array<uintptr_t, 32> ListEntry1{};
    std::array<uintptr_t, 32> EntityController{};
    std::array<uint32_t, 32> EntityControllerPawn{};
    std::array<uintptr_t, 32> ListEntry2{};
    std::array<uintptr_t, 32> EntityPawn{};
    std::array<uint8_t, 32> Team{};
    std::array<int, 32> Health{};
    std::array<glm::vec3, 32> Origin{};
    std::array<std::array<char, 128>, 32> NameBuffer{};
    std::array<uintptr_t, 32> GameSceneNode{};
    std::array<uintptr_t, 32> BoneArray{};
    std::array<BoneMatrix, 32> BoneMatrices{};
};

class Cheat
{
  public:
    Cheat() = default;
    ~Cheat();

    auto Attach(const std::string &processName) -> void;
    void Detach();
    bool IsAttached() const;

    void OnUpdate();

    bool WorldToScreen(const glm::vec3 &pos, glm::vec3 &screen, const glm::mat4 &viewMatrix) const;

    void RenderEsp(const std::vector<EntityData> &entities) const;
    bool ToggleEsp() const
    {
        return m_bEsp;
    }
    bool ToggleTeamEsp() const
    {
        return m_bTeamEsp;
    }

    DWORD GetProcessId() const
    {
        return m_dwProcessId;
    }
    uintptr_t GetBaseAddress() const
    {
        return m_baseAddress;
    }

    std::vector<EntityData> GetEntities() const
    {
        std::scoped_lock lock(m_dataMutex);
        return m_entities;
    }

  private:
    void GetEntityData();
    void WorkerLoop();

  public:
    bool m_bAimbot = false;
    bool m_bEsp = false;
    bool m_bTeamEsp = false;
    bool m_bBunnyHop = false;

    // Granular ESP feature toggles
    bool m_bBox = true;
    bool m_bBones = true;
    bool m_bName = true;
    bool m_bHealth = true;
    bool m_bDistance = true;
    bool m_bSnaplines = false;

    std::string m_gameName;

  private:
    float m_width = 0.f, m_height = 0.f;
    DWORD m_dwProcessId = 0;
    uintptr_t m_baseAddress = 0;

    // Deadlock-specific: resolved at attach time via pattern scanning
    DeadlockOffsets m_dlOffsets;

    LocalPlayerData m_localPlayerData;
    uintptr_t m_entityList = 0;
    glm::mat4 m_viewMatrix{};

    // Pipeline scratch data (heap-allocated as class member instead of stack)
    EntityPipelineData m_pipeline;

    // Double-buffer: worker writes to m_backBuffer, render reads from m_entities
    std::vector<EntityData> m_entities;
    std::vector<EntityData> m_backBuffer;
    mutable std::mutex m_dataMutex;

    // Worker thread
    std::thread m_workerThread;
    std::atomic<bool> m_running{false};

    VMMDLL_SCATTER_HANDLE m_scatterHandle{};
};
