#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include <vector>

class Scene;
class Viewport;

class SystemPipeline
{
private:
	std::vector<Ref<ISystem>> m_Systems;

	Ref<Scene> m_Scene;
public:
	SystemPipeline(Ref<Scene> m_Scene, Viewport& viewport);
	~SystemPipeline();

	void Update();
};
