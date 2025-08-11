#pragma once
#include "core/Base.h"
#include "RenderingSystem.h"
#include "MeshGeneratingSystem.h"

class Scene;

class SystemPipeline
{
private:
	Ref<RenderingSystem> m_RenderingSystem;
	Ref<MeshGeneratingSystem> m_MeshGeneratingSystem;

	Ref<Scene> m_Scene;
public:
	SystemPipeline(Ref<Scene> m_Scene);
	~SystemPipeline();

	void Update();
};

