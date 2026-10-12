#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include <vector>

class Scene;
class Viewport;

class SystemPipeline
{
protected:
	std::vector<Unique<ISystem>> m_Systems;
public:
	virtual ~SystemPipeline() = default;
	void Update();
};

class ModellingSystemPipeline : public SystemPipeline
{
public:
	ModellingSystemPipeline(Ref<Scene> m_Scene, Viewport& viewport);
};

class CAMSystemPipeline : public SystemPipeline
{
public:
	CAMSystemPipeline(Ref<Scene> scene, Viewport& viewport);
};
