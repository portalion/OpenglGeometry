#include "HeightMap.h"

void HeightMap::Reset()
{
	m_HeightMap->Upload(m_CurrentHeight);
}

HeightMap::HeightMap()
{
	m_HeightMap = CreateRef<Texture2D>(0, 0, TextureFormat::R32F);
}

void HeightMap::ChangeSize(unsigned int width, unsigned int height, float value)
{
	if (m_CurrentHeight == value && m_HeightMap->Width() == width && m_HeightMap->Height() == height)
	{
		return;
	}

	m_CurrentHeight = value;
	m_HeightMap->Resize(width, height);
	m_HeightMap->Upload(value);
}
