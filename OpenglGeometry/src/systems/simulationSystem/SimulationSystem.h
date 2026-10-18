#pragma once
#include <vector>
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include "scene/CAMSettings.h"
#include "HeightMap.h"

class Scene;

class SimulationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<CAMSettings> m_Settings;

	HeightMap m_HeightMap;

	void ResetMaterial();
public:
	SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings);

	void Process() override;
};
