#pragma once

#include <Windows.h>

class Drawing
{
  public:
    static bool IsActive();
    static auto SetActive(bool bActive) -> void;

    static bool m_bActive;
};