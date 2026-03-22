#include "Drawing.h"

bool Drawing::m_bActive = true;

bool Drawing::IsActive()
{
    return m_bActive;
}

auto Drawing::SetActive(bool bActive) -> void
{
    m_bActive = bActive;
}