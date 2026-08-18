#include "Viewport.h"
#include <utils/GlCall.h>

float Viewport::Aspect() const
{
	return IsValid() ? static_cast<float>(m_Data.width) / static_cast<float>(m_Data.height) : 1.f;
}

void Viewport::Change(ViewportData newData)
{
	if (newData == m_Data)
	{
		return;
	}

	m_Data = newData;
	m_Dirty = true;
}

void Viewport::Apply()
{
	m_Dirty = false;
	if (IsValid())
	{
		GLCall(glViewport(m_Data.x, m_Data.y, m_Data.width, m_Data.height));
	}
}
