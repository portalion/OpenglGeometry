#include "ToolModel.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numbers>
#include "scene/Scene.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "renderer/VertexArray.h"

namespace
{
	constexpr int CircleSegments = 24;
	constexpr int SideLines = 8;
	constexpr int SphereRings = 4;
	constexpr float ShaftLength = 50.f;

	Algebra::Quaternion PathRotation()
	{
		// Same rotation the path archetype uses, so the tool lines up with the drawn paths
		return Algebra::Quaternion::CreateFromAxisAngle({ 1.f, 0.f, 0.f, 0.f }, std::numbers::pi_v<float> / 2.f);
	}
}

ToolModel::ToolModel(Ref<Scene> scene)
{
	m_Entity = scene->CreateEntity();
	m_Entity.AddComponent<PositionComponent>();
	m_Entity.AddComponent<RotationComponent>().rotation = PathRotation();
	m_Entity.AddComponent<ColorComponent>().color = { 0.2f, 0.6f, 1.f, 1.f };

	auto& meshComponent = m_Entity.AddComponent<MeshComponent>();
	meshComponent.renderingMode = RenderingMode::Lines;
	meshComponent.shaderTypes = { AvailableShaders::Default };

	m_Entity.AddTag<IsInvisibleTag>();
	m_Visible = false;
}

void ToolModel::Rebuild(const Tool& tool)
{
	m_Diameter = tool.m_Diameter;
	m_IsFlat = tool.m_IsFlat;
	m_CuttingLength = tool.m_CuttingLength;

	const float radius = tool.m_Diameter / 2.f;
	const float angleStep = 2.f * std::numbers::pi_v<float> / CircleSegments;

	std::vector<Algebra::Vector4> vertices;
	std::vector<uint32_t> indices;

	auto addCircle = [&](float z, float circleRadius)
	{
		const auto start = static_cast<uint32_t>(vertices.size());
		for (int i = 0; i < CircleSegments; i++)
		{
			vertices.emplace_back(circleRadius * std::cos(angleStep * i), circleRadius * std::sin(angleStep * i), z, 1.f);
			indices.insert(indices.end(), { start + i, start + (i + 1) % CircleSegments });
		}
	};

	auto addLine = [&](const Algebra::Vector4& a, const Algebra::Vector4& b)
	{
		const auto start = static_cast<uint32_t>(vertices.size());
		vertices.push_back(a);
		vertices.push_back(b);
		indices.insert(indices.end(), { start, start + 1 });
	};

	// Ball cutters have a hemisphere at the tip, flat cutters start the shaft right at the tip
	const float shaftStart = m_IsFlat ? 0.f : radius;
	const float cuttingEnd = std::max(m_CuttingLength, shaftStart);
	const float shaftEnd = std::max(shaftStart + ShaftLength, cuttingEnd + radius);

	addCircle(shaftStart, radius);
	addCircle(cuttingEnd, radius);
	addCircle(shaftEnd, radius);
	for (int i = 0; i < SideLines; i++)
	{
		const float angle = 2.f * std::numbers::pi_v<float> * i / SideLines;
		const float x = radius * std::cos(angle);
		const float y = radius * std::sin(angle);
		addLine({ x, y, shaftStart, 1.f }, { x, y, shaftEnd, 1.f });

		// Slanted flute lines mark the cutting part
		const float nextAngle = 2.f * std::numbers::pi_v<float> * (i + 1) / SideLines;
		addLine({ x, y, shaftStart, 1.f }, { radius * std::cos(nextAngle), radius * std::sin(nextAngle), cuttingEnd, 1.f });

		if (!m_IsFlat)
		{
			// Meridian from the tip to the equator of the hemisphere
			for (int j = 0; j < SphereRings * 2; j++)
			{
				const float a0 = std::numbers::pi_v<float> / 2.f * j / (SphereRings * 2);
				const float a1 = std::numbers::pi_v<float> / 2.f * (j + 1) / (SphereRings * 2);
				addLine({ x * std::sin(a0), y * std::sin(a0), radius - radius * std::cos(a0), 1.f },
					{ x * std::sin(a1), y * std::sin(a1), radius - radius * std::cos(a1), 1.f });
			}
		}
	}

	if (m_IsFlat)
	{
		addLine({ -radius, 0.f, 0.f, 1.f }, { radius, 0.f, 0.f, 1.f });
		addLine({ 0.f, -radius, 0.f, 1.f }, { 0.f, radius, 0.f, 1.f });
	}
	else
	{
		for (int j = 1; j < SphereRings; j++)
		{
			const float angle = std::numbers::pi_v<float> / 2.f * j / SphereRings;
			addCircle(radius - radius * std::cos(angle), radius * std::sin(angle));
		}
	}

	m_Entity.GetComponent<MeshComponent>().mesh = VertexArray::CreateWithBuffers(vertices, indices,
		{ { ShaderDataType::Float4, "a_Position" } });
}

void ToolModel::Update(const Tool& tool, const Algebra::Vector4& tipPosition)
{
	if (tool.m_Diameter != m_Diameter || tool.m_IsFlat != m_IsFlat || tool.m_CuttingLength != m_CuttingLength || !m_Entity.GetComponent<MeshComponent>().mesh)
	{
		Rebuild(tool);
	}

	// Model matrix is T * R * S, so the tip has to be rotated the same way as the mesh
	Algebra::Vector4 position = PathRotation().ToMatrix() * Algebra::Vector4(tipPosition.x, tipPosition.y, tipPosition.z, 1.f);
	m_Entity.GetComponent<PositionComponent>().position = position;
}

void ToolModel::SetVisible(bool visible)
{
	if (visible == m_Visible)
	{
		return;
	}
	m_Visible = visible;

	if (visible)
	{
		m_Entity.RemoveTag<IsInvisibleTag>();
	}
	else
	{
		m_Entity.AddTag<IsInvisibleTag>();
	}
}
