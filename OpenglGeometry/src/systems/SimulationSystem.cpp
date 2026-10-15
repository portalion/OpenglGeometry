#include "SimulationSystem.h"
#include "scene/Scene.h"

SimulationSystem::SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings)
	:m_Scene(scene), m_Settings(settings)
{
}

void SimulationSystem::Process()
{
}
