#include "SimulationSystem.h"
#include <algorithm>
#include "scene/Scene.h"
#include "renderer/Texture2D.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include <limits>

SimulationSystem::SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings)
	:m_Scene(scene), m_Settings(settings), m_Block(scene, m_HeightMap.Texture())
{
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

	m_PathIndex = 0;
	m_SegmentIndex = 0;
	m_SegmentDistance = 0.f;
}

bool SimulationSystem::Advance(float distance)
{
	const auto& material = m_Settings->m_Material;
	const float sizeX = material.m_Size[0];
	const float sizeY = material.m_Size[1];
	// Step about one texel at a time so consecutive cuts overlap without gaps.
	const float step = std::min(sizeX / material.m_Resolution[0], sizeY / material.m_Resolution[1]);

	std::vector<Entity> paths;
	for (Entity entity : m_Scene->GetAllEntitiesWith<PathComponent>())
	{
		paths.push_back(entity);
	}

	while (m_PathIndex < paths.size())
	{
		const auto& path = paths[m_PathIndex].GetComponent<PathComponent>();
		const float radius = 0.5f * path.m_Tool.m_Diameter;

		while (m_SegmentIndex + 1 < path.m_Positions.size())
		{
			const auto& from = path.m_Positions[m_SegmentIndex];
			const auto& to = path.m_Positions[m_SegmentIndex + 1];
			const float length = (to - from).Length();

			while (m_SegmentDistance <= length)
			{
				if (distance <= 0.f)
				{
					return false;
				}

				const float t = length > 0.f ? m_SegmentDistance / length : 1.f;
				auto tool = from + (to - from) * t;
				tool.w = radius;
				m_HeightMap.Mill(tool, path.m_Tool.m_IsFlat, sizeX, sizeY);

				m_SegmentDistance += step;
				distance -= step;
			}

			m_SegmentDistance -= length;
			m_SegmentIndex++;
		}

		m_SegmentIndex = 0;
		m_SegmentDistance = 0.f;
		m_PathIndex++;
	}

	return true;
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

	const auto now = std::chrono::steady_clock::now();
	const float elapsed = std::chrono::duration<float>(now - m_LastStep).count();
	m_LastStep = now;

	if (simulation.m_FinishRequested)
	{
		simulation.m_FinishRequested = false;
		Advance(std::numeric_limits<float>::infinity());
		simulation.m_State = SimulationState::Finished;
		simulation.m_Progress = 1.f;
	}
	else if (simulation.m_State == SimulationState::Running && Advance(simulation.m_Speed * elapsed))
	{
		simulation.m_State = SimulationState::Finished;
		simulation.m_Progress = 1.f;
	}
}
