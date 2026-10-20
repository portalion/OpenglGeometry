#pragma once
#include "core/Base.h"
#include <renderer/Texture2D.h>
#include <Algebra.h>

class HeightMap
{
private:
	bool m_HeightsDirty = false;
	Ref<Texture2D> m_HeightMap;
	float m_CurrentHeight = 0.f;

public:
	HeightMap();
	void Reset();
	void ChangeSize(unsigned int width, unsigned int height, float value);
	inline unsigned int Width() const { return m_HeightMap->Width(); }
	inline unsigned int Height() const { return m_HeightMap->Height(); }
	inline float InitialHeight() const { return m_CurrentHeight; }
	inline Ref<Texture2D> Texture() const { return m_HeightMap; }

	// Lowers the material under one tool position; tool.xyz is the tip [mm], tool.w the radius [mm].
	void Mill(const Algebra::Vector4& tool, bool isFlat, float sizeX, float sizeY);
};