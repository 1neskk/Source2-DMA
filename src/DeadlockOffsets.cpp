#include "DeadlockOffsets.h"

#include <iostream>

#include "DMALibrary/Memory/Memory.h"

// ---- dwEntityList ----------------------------------------------------------
// Unique code sequence that reads g_pEntitySystem (entity list) via RIP-rel.
// Pattern: mov rax, [rip+?] ; mov eax, [rax+10]
static constexpr const char* SIG_ENTITY_LIST =
    "48 8B 0D ? ? ? ? 48 85 C9 74 ? 8B 81";

// ---- dwLocalPlayerController -----------------------------------------------
// Unique sequence referencing the local player controller global.
// Pattern: mov rcx, [rip+?] ; test rcx, rcx ; jz …
static constexpr const char* SIG_LOCAL_PLAYER_CTRL =
    "48 8B 05 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 88";

// ---- dwViewMatrix ----------------------------------------------------------
// Source2 view matrix (4×4 float, 64 bytes).
// Pattern: lea rax, [rip+?] ; mov [r??+?], rax  (ViewMatrix assignment)
static constexpr const char* SIG_VIEW_MATRIX =
    "48 8D ? ? ? ? ? 48 C1 E0 06 48 03 C1 C3";

// ---------------------------------------------------------------------------
// ResolveRip
// ---------------------------------------------------------------------------
// Given the address of the first byte of an instruction (as returned by
// FindSignature), read the 32-bit signed displacement stored at
// (sigAddr + instrOffset) and compute the absolute target address:
//
//   targetVA = (sigAddr + instrTotalLen) + disp32
//
// This mirrors how the CPU decodes RIP-relative addressing.
// ---------------------------------------------------------------------------
uintptr_t DeadlockOffsetGenerator::ResolveRip(uint64_t sigAddr, int instrOffset, int instrTotalLen)
{
    if (!sigAddr)
        return 0;

    int32_t disp = 0;
    if (!mem.Read(sigAddr + instrOffset, &disp, sizeof(disp)))
        return 0;

    const uintptr_t target = static_cast<uintptr_t>(
        static_cast<int64_t>(sigAddr + instrTotalLen) + disp);

    return target;
}

// ---------------------------------------------------------------------------
// Scan
// ---------------------------------------------------------------------------
bool DeadlockOffsetGenerator::Scan(uintptr_t moduleBase, size_t moduleSize,
                                    DWORD pid, DeadlockOffsets& out)
{
    const uint64_t base = moduleBase;
    const uint64_t end  = base + moduleSize;

    std::cout << "[Deadlock] Scanning client.dll (base=0x" << std::hex
              << base << ", size=0x" << moduleSize << std::dec << ")...\n";

    // ── dwEntityList ───────────────────────────────────────────────────────
    {
        const uint64_t sigAddr = mem.FindSignature(SIG_ENTITY_LIST, base, end, static_cast<int>(pid));
        // Instruction: 48 8B 0D [? ? ? ?]  = mov rcx, [rip+disp32]  (7 bytes)
        out.dwEntityList = ResolveRip(sigAddr, /*instrOffset=*/3, /*instrTotalLen=*/7);

        std::cout << "[Deadlock] dwEntityList            = 0x"
                  << std::hex << out.dwEntityList << std::dec;
        if (!out.dwEntityList)
            std::cout << "  [!] NOT FOUND - pattern may need updating";
        std::cout << '\n';
    }

    // ── dwLocalPlayerController ────────────────────────────────────────────
    {
        const uint64_t sigAddr = mem.FindSignature(SIG_LOCAL_PLAYER_CTRL, base, end, static_cast<int>(pid));
        // Instruction: 48 8B 05 [? ? ? ?]  = mov rax, [rip+disp32]  (7 bytes)
        out.dwLocalPlayerController = ResolveRip(sigAddr, /*instrOffset=*/3, /*instrTotalLen=*/7);

        std::cout << "[Deadlock] dwLocalPlayerController = 0x"
                  << std::hex << out.dwLocalPlayerController << std::dec;
        if (!out.dwLocalPlayerController)
            std::cout << "  [!] NOT FOUND - pattern may need updating";
        std::cout << '\n';
    }

    // ── dwViewMatrix ───────────────────────────────────────────────────────
    {
        const uint64_t sigAddr = mem.FindSignature(SIG_VIEW_MATRIX, base, end, static_cast<int>(pid));
        // Instruction: 48 8D 05 [? ? ? ?]  = lea rax, [rip+disp32]  (7 bytes)
        out.dwViewMatrix = ResolveRip(sigAddr, /*instrOffset=*/3, /*instrTotalLen=*/7);

        std::cout << "[Deadlock] dwViewMatrix            = 0x"
                  << std::hex << out.dwViewMatrix << std::dec;
        if (!out.dwViewMatrix)
            std::cout << "  [!] NOT FOUND - pattern may need updating";
        std::cout << '\n';
    }

    if (out.IsValid())
    {
        std::cout << "[Deadlock] All offsets resolved successfully.\n";
        return true;
    }

    std::cout << "[Deadlock] WARNING: One or more offsets could not be resolved.\n"
              << "[Deadlock] The cheat will fall back to the last known hardcoded offsets.\n";
    return false;
}
