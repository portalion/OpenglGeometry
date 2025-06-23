#pragma once
#include "core/Base.h"

class Scene;

class SystemPipeline
{
private:
	Ref<Scene> scene;
public:
	SystemPipeline(Ref<Scene> scene);
	~SystemPipeline();

	void Update();
};

