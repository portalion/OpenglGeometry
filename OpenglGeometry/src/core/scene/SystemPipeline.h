#pragma once
#include "core/Base.h"
#include "systems/RenderingSystem.h"

class Scene;

class SystemPipeline
{
private:
	Ref<RenderingSystem> renderingSystem;

	Ref<Scene> scene;
public:
	SystemPipeline(Ref<Scene> scene);
	~SystemPipeline();

	void Update();
};

