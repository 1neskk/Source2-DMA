#pragma once

#include <cstddef>
#include <cstdint>

struct DeadlockOffsets
{
    // ── Module-level globals (resolved at runtime via pattern scanning) ───
    uintptr_t dwEntityList            = 0;
    uintptr_t dwLocalPlayerController = 0;
    uintptr_t dwViewMatrix            = 0;

    // ── CBaseEntity member offsets ────────────────────────────────────────
    uintptr_t m_iTeamNum              = 0x3B3;
    uintptr_t m_iHealth               = 0x330;
    uintptr_t m_pGameSceneNode        = 0x298;

    // ── CBasePlayerController member offsets ─────────────────────────────
    uintptr_t m_hPlayerPawn           = 0x8A4;   // handle to controlled pawn
    uintptr_t m_iszPlayerName         = 0x640;   // player name string (max 128 chars)

    // ── CBasePlayerPawn member offsets ───────────────────────────────────
    uintptr_t m_vOldOrigin            = 0x105C;  // world-space position (vec3)

    // ── Returns true when all critical globals have been resolved ─────────
    [[nodiscard]] bool IsValid() const noexcept
    {
        return dwEntityList && dwLocalPlayerController && dwViewMatrix;
    }
};
