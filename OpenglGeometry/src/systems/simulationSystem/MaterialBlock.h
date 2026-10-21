#pragma once
#include <array>
#include "core/Base.h"
#include "scene/Entity.h"

class Scene;
class Texture2D;

class MaterialBlock
{
private:
	Entity m_Entity;
	int m_ResolutionX = 0;
	int m_ResolutionY = 0;

public:
	MaterialBlock(Ref<Scene> scene, Ref<Texture2D> heightMap);

	void Rebuild(int resolutionX, int resolutionY);
	void SetSize(const std::array<float, 3>& size);
};
