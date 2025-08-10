#pragma once
#include "core/Base.h"
#include "systems/RenderingSystem.h"

class Scene;

class SystemPipeline
{
private:
	Ref<RenderingSystem> renderingSystem;

	Ref<Scene> m_Scene;
public:
	SystemPipeline(Ref<Scene> m_Scene);
	~SystemPipeline();

	void Update();
};

