#include "SimulationSystem.h"
#include <algorithm>
#include "scene/Scene.h"
#include "renderer/Texture2D.h"

SimulationSystem::SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings)
	:m_Scene(scene), m_Settings(settings), m_Block(scene, m_HeightMap.Texture())
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
	m_Block.Rebuild(resX, resY);
	m_Block.SetSize(material.m_Size);
}

void SimulationSystem::Process()
{
	auto& material = m_Settings->m_Material;
	auto& simulation = m_Settings->m_Simulation;

	const bool materialChanged = m_HeightMap.Width() != material.m_Resolution[0]
		|| m_HeightMap.Height() != material.m_Resolution[1]
		|| m_HeightMap.InitialHeight() != material.m_Size[2];

	m_Block.SetSize(material.m_Size);

	if (simulation.m_ResetRequested || (materialChanged && simulation.m_State == SimulationState::Stopped))
	{
		ResetMaterial();
		simulation.m_ResetRequested = false;
	}
}
