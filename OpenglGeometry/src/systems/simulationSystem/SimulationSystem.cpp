#include "SimulationSystem.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "scene/Scene.h"
#include "renderer/Texture2D.h"
#include "managers/ShaderManager.h"
#include "scene/Components.h"

SimulationSystem::SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings)
	:m_Scene(scene), m_Settings(settings), m_Block(scene, m_HeightMap.Texture()), m_ToolModel(scene)
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
		m_ToolPathId.reset();
		m_ToolPlaced = false;
		simulation.m_ResetRequested = false;
	}

	Simulate(TickFrameTime());
	UpdateToolModel();
}

void SimulationSystem::Simulate(float deltaTime)
{
	auto& simulation = m_Settings->m_Simulation;

	const bool finishRequested = simulation.m_FinishRequested;
	simulation.m_FinishRequested = false;

	const bool advancing = simulation.m_State == SimulationState::Running
		|| (finishRequested && simulation.m_State == SimulationState::Paused);
	if (!advancing)
	{
		return;
	}

	const auto* pathComponent = FindActivePath();
	if (!pathComponent || pathComponent->m_Positions.size() < 2)
	{
		return;
	}
	const auto* path = &pathComponent->m_Positions;

	if (m_ToolPathId != simulation.m_ActivePath)
	{
		ResetTool(*path);
		m_ToolPathId = simulation.m_ActivePath;
	}

	const float distance = finishRequested ? std::numeric_limits<float>::infinity() : simulation.m_Speed * deltaTime;
	const auto sweptPoints = AdvanceTool(*path, distance);
	ProcessHeightMap(sweptPoints, pathComponent->m_Tool);

	simulation.m_Progress = m_PathLength > 0.f ? m_TravelledLength / m_PathLength : 1.f;
	if (m_ToolSegment + 1 >= path->size())
	{
		simulation.m_State = SimulationState::Finished;
		simulation.m_Progress = 1.f;
		// Starting this path again begins from its first point
		m_ToolPathId.reset();
	}
}

void SimulationSystem::UpdateToolModel()
{
	const auto* pathComponent = FindActivePath();
	const bool visible = m_Settings->m_Simulation.m_ShowTool && pathComponent && m_ToolPlaced;

	m_ToolModel.SetVisible(visible);
	if (visible)
	{
		m_ToolModel.Update(pathComponent->m_Tool, m_ToolPosition);
	}
}

float SimulationSystem::TickFrameTime()
{
	constexpr float maxDeltaTime = 0.25f;

	const auto now = std::chrono::steady_clock::now();
	const float deltaTime = std::chrono::duration<float>(now - m_LastFrame).count();
	m_LastFrame = now;
	return std::min(deltaTime, maxDeltaTime);
}

const PathComponent* SimulationSystem::FindActivePath() const
{
	const auto& activePath = m_Settings->m_Simulation.m_ActivePath;
	for (auto entity : m_Scene->GetAllEntitiesWith<PathComponent>())
	{
		if (entity.GetComponent<IdComponent>().id == activePath)
		{
			return &entity.GetComponent<PathComponent>();
		}
	}
	return nullptr;
}

namespace
{
	float Distance(const Algebra::Vector4& a, const Algebra::Vector4& b)
	{
		const Algebra::Vector4 d = b - a;
		return std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
	}
}

void SimulationSystem::ResetTool(const std::vector<Algebra::Vector4>& path)
{
	m_ToolSegment = 0;
	m_ToolSegmentOffset = 0.f;
	m_TravelledLength = 0.f;
	m_PathLength = 0.f;
	m_ToolPosition = path.front();
	m_ToolPlaced = true;
	for (size_t i = 0; i + 1 < path.size(); i++)
	{
		m_PathLength += Distance(path[i], path[i + 1]);
	}
}

std::vector<Algebra::Vector4> SimulationSystem::AdvanceTool(const std::vector<Algebra::Vector4>& path, float distance)
{
	auto toolPosition = [&]()
	{
		const auto& a = path[m_ToolSegment];
		const auto& b = path[m_ToolSegment + 1];
		const float length = Distance(a, b);
		return length > 0.f ? a + (b - a) * (m_ToolSegmentOffset / length) : a;
	};

	std::vector<Algebra::Vector4> sweptPoints{ toolPosition() };

	while (m_ToolSegment + 1 < path.size())
	{
		const float remaining = Distance(path[m_ToolSegment], path[m_ToolSegment + 1]) - m_ToolSegmentOffset;
		if (distance < remaining)
		{
			m_ToolSegmentOffset += distance;
			m_TravelledLength += distance;
			sweptPoints.push_back(toolPosition());
			break;
		}

		distance -= remaining;
		m_TravelledLength += remaining;
		m_ToolSegment++;
		m_ToolSegmentOffset = 0.f;
		sweptPoints.push_back(path[m_ToolSegment]);
	}

	m_ToolPosition = sweptPoints.back();
	return sweptPoints;
}

void SimulationSystem::ProcessHeightMap(const std::vector<Algebra::Vector4>& sweptPoints, const Tool& tool)
{
	constexpr unsigned int groupSize = 16; 	
	constexpr unsigned int pathBinding = 0;
	if (sweptPoints.size() < 2)
	{
		return;
	}

	std::vector<Algebra::Vector4> vectors;
	vectors.reserve(sweptPoints.size());
	for (const auto& point : sweptPoints)
	{
		vectors.emplace_back(point.x, point.z, point.y, 1.f);
	}
	m_PathBuffer.SetData(vectors.data(), static_cast<uint32_t>(vectors.size() * sizeof(Algebra::Vector4)));

	const auto& material = m_Settings->m_Material;

	auto shader = ShaderManager::GetInstance().GetShader(
		tool.m_IsFlat ? AvailableShaders::HeightMapCarve : AvailableShaders::HeightMapCarveBall);

	UniformContext context;
	context.TextureUniforms["heightmap"] = m_HeightMap.Texture();
	context.UIntUniforms["vectorCount"] = static_cast<unsigned int>(vectors.size());
	context.UIntUniforms["cutterRadius"] = static_cast<unsigned int>(tool.m_Diameter / 2.f);
	context.IntUniforms["nx"] = material.m_Resolution[0];
	context.IntUniforms["nz"] = material.m_Resolution[1];
	context.FloatUniforms["sizeX"] = material.m_Size[0];
	context.FloatUniforms["sizeZ"] = material.m_Size[1];

	shader->Bind();
	shader->ApplyContext(context);
	m_PathBuffer.BindBase(pathBinding);
	shader->Dispatch((m_HeightMap.Width() + groupSize - 1) / groupSize, (m_HeightMap.Height() + groupSize - 1) / groupSize);
	shader->UnBind();
}
