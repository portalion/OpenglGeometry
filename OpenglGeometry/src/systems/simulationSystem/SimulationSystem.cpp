#include "SimulationSystem.h"
#include <algorithm>
#include "scene/Scene.h"
#include "renderer/Texture2D.h"

SimulationSystem::SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings)
	:m_Scene(scene), m_Settings(settings)
{
	const auto& resolution = m_Settings->m_Material.m_Resolution;
	ResetMaterial();
}

void SimulationSystem::ResetMaterial()
{
	const auto& material = m_Settings->m_Material;
	const int resX = material.m_Resolution[0];
	const int resY = material.m_Resolution[1];

	m_HeightMap.ChangeSize(resX, resY, material.m_Size[2]);
	m_HeightMap.Reset();
}

void SimulationSystem::Process()
{
	auto& material = m_Settings->m_Material;
	auto& simulation = m_Settings->m_Simulation;

	const bool resolutionChanged = m_HeightMap.Width() != material.m_Resolution[0]
		|| m_HeightMap.Height() != material.m_Resolution[1];

	if (simulation.m_ResetRequested || (resolutionChanged && simulation.m_State == SimulationState::Stopped))
	{
		ResetMaterial();
		simulation.m_ResetRequested = false;
	}
}
