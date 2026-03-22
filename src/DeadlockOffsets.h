#pragma once

#include <cstdint>
#include <Windows.h>

#include "offsets/deadlock_offsets.hpp"

class DeadlockOffsetGenerator
{
public:
    // Scan client.dll and fill |out|.  Returns true if all three critical
    // globals were successfully resolved.
    static bool Scan(uintptr_t moduleBase, size_t moduleSize, DWORD pid,
                     DeadlockOffsets& out);

private:
    // Read the 4-byte RIP-relative displacement at (sigAddr + instrOffset)
    // and return the absolute target address.
    //   instrOffset  : byte offset from sigAddr to the start of the disp32
    //   instrTotalLen: total length of the instruction (displacement is the
    //                  last 4 bytes before the next instruction begins)
    static uintptr_t ResolveRip(uint64_t sigAddr, int instrOffset, int instrTotalLen);
};
