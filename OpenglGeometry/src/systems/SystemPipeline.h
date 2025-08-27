#pragma once
#include "core/Base.h"
#include "ISystem.h"
#include <vector>

class Scene;

class SystemPipeline
{
private:
	std::vector<Ref<ISystem>> m_Systems;

	Ref<Scene> m_Scene;
public:
	SystemPipeline(Ref<Scene> m_Scene);
	~SystemPipeline();

	void Update();
};

