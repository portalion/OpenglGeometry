#pragma once
#include "core/Base.h"
#include <renderer/Texture2D.h>

class HeightMap
{
private:
	bool m_HeightsDirty = false;
	Ref<Texture2D> m_HeightMap;
	float m_CurrentHeight;

public:
	HeightMap();
	void Reset();
	void ChangeSize(unsigned int width, unsigned int height, float value);
	inline unsigned int Width() const { return m_HeightMap->Width(); }
	inline unsigned int Height() const { return m_HeightMap->Height(); }
};