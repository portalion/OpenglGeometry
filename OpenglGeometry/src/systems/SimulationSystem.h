#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include "scene/CAMSettings.h"

class Scene;

class SimulationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<CAMSettings> m_Settings;
public:
	SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings);

	void Process() override;
};
