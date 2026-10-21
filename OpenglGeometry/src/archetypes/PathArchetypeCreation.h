#pragma once
#include "SimpleArchetypeCreation.h"
#include <renderer/VertexArray.h>
#include <numbers>

namespace Archetypes
{
	inline void AddPathToEntity(Entity& entity, PathComponent pathArgs)
	{
		const auto id = entity.AddComponent<IdComponent>().id;
		entity.AddComponent<NameComponent>().name = "Path " + std::to_string(id);
		auto& pathComponent = entity.AddComponent<PathComponent>();
		pathComponent.m_Tool = pathArgs.m_Tool;
		pathComponent.m_Positions = pathArgs.m_Positions;

		std::vector<unsigned int> indices;
		indices.reserve(pathArgs.m_Positions.size() * 2);
		for (int i = 0; i + 1 < static_cast<int>(pathArgs.m_Positions.size()); i++)
		{
			indices.push_back(i);
			indices.push_back(i + 1);
		}

		auto vertexArray = VertexArray::CreateWithBuffers(pathComponent.m_Positions, indices,
			{{ ShaderDataType::Float4, "a_Position" }});

		auto& meshComponent = entity.AddComponent<MeshComponent>();
		meshComponent.renderingMode = RenderingMode::Lines;
		meshComponent.shaderTypes = { AvailableShaders::Default };
		meshComponent.mesh = vertexArray;

		entity.AddComponent<ColorComponent>().color = {0.f, 1.f, 0.f, 1.f};

		entity.AddComponent<RotationComponent>().rotation =
			Algebra::Quaternion::CreateFromAxisAngle({ 1.f, 0.f, 0.f, 0.f }, std::numbers::pi_v<float> / 2.f);
	}
}
