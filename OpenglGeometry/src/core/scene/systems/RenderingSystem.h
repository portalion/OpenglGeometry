#pragma once
#include "core/Base.h"

class Scene;

class RenderingSystem
{
private:
	Ref<Scene> scene;


public:
	RenderingSystem(Ref<Scene> scene);

	void Process();
};

