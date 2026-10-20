#include "MaterialBlock.h"
#include <vector>
#include "scene/Scene.h"
#include "scene/Components.h"
#include "renderer/VertexArray.h"

namespace
{
	// Vertex layout: (u, v, isTop, normalCode).
	// u, v in [0, 1] span the block along material X / Y.
	// isTop = 1 means the vertex is lifted to the height map, 0 means it stays at the bottom.
	// normalCode: 0 = from height map, 1 = -X, 2 = +X, 3 = -Y, 4 = +Y, 5 = bottom.
	enum NormalCode
	{
		FromHeightMap = 0,
		NegativeX = 1,
		PositiveX = 2,
		NegativeY = 3,
		PositiveY = 4,
		Bottom = 5
	};

	void AddQuad(std::vector<uint32_t>& indices, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
	{
		indices.insert(indices.end(), { a, b, c, a, c, d });
	}

	// Wall along an edge of the top grid: a top row following the height map and a bottom row.
	void AddWall(std::vector<Algebra::Vector4>& vertices, std::vector<uint32_t>& indices,
		int count, float normalCode, auto edgeUV)
	{
		const uint32_t start = static_cast<uint32_t>(vertices.size());
		for (int i = 0; i < count; i++)
		{
			const auto [u, v] = edgeUV(i);
			vertices.emplace_back(u, v, 1.f, normalCode);
			vertices.emplace_back(u, v, 0.f, normalCode);
		}
		for (int i = 0; i + 1 < count; i++)
		{
			const uint32_t top = start + 2 * i;
			AddQuad(indices, top, top + 1, top + 3, top + 2);
		}
	}
}

MaterialBlock::MaterialBlock(Ref<Scene> scene, Ref<Texture2D> heightMap)
{
	m_Entity = scene->CreateEntity();
	m_Entity.AddComponent<PositionComponent>();
	m_Entity.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };
	m_Entity.AddComponent<ColorComponent>().color = { 0.85f, 0.75f, 0.55f, 1.f };
	m_Entity.AddComponent<HeightMapComponent>().texture = heightMap;

	auto& meshComponent = m_Entity.AddComponent<MeshComponent>();
	meshComponent.renderingMode = RenderingMode::Triangles;
	meshComponent.shaderTypes = { AvailableShaders::HeightMap };
}

void MaterialBlock::Rebuild(int resolutionX, int resolutionY)
{
	if (resolutionX == m_ResolutionX && resolutionY == m_ResolutionY)
	{
		return;
	}
	m_ResolutionX = resolutionX;
	m_ResolutionY = resolutionY;

	const float stepU = 1.f / (resolutionX - 1);
	const float stepV = 1.f / (resolutionY - 1);

	std::vector<Algebra::Vector4> vertices;
	std::vector<uint32_t> indices;
	vertices.reserve(static_cast<size_t>(resolutionX) * resolutionY + 4 * (resolutionX + resolutionY) + 4);
	indices.reserve(static_cast<size_t>(resolutionX - 1) * (resolutionY - 1) * 6
		+ 12 * (resolutionX + resolutionY) + 6);

	// top layer, one vertex per height map texel
	for (int j = 0; j < resolutionY; j++)
	{
		for (int i = 0; i < resolutionX; i++)
		{
			vertices.emplace_back(i * stepU, j * stepV, 1.f, static_cast<float>(FromHeightMap));
		}
	}
	for (int j = 0; j + 1 < resolutionY; j++)
	{
		for (int i = 0; i + 1 < resolutionX; i++)
		{
			const uint32_t corner = j * resolutionX + i;
			AddQuad(indices, corner, corner + 1, corner + resolutionX + 1, corner + resolutionX);
		}
	}

	AddWall(vertices, indices, resolutionX, NegativeY, [&](int i) { return std::pair{ i * stepU, 0.f }; });
	AddWall(vertices, indices, resolutionX, PositiveY, [&](int i) { return std::pair{ i * stepU, 1.f }; });
	AddWall(vertices, indices, resolutionY, NegativeX, [&](int j) { return std::pair{ 0.f, j * stepV }; });
	AddWall(vertices, indices, resolutionY, PositiveX, [&](int j) { return std::pair{ 1.f, j * stepV }; });

	const uint32_t bottom = static_cast<uint32_t>(vertices.size());
	vertices.emplace_back(0.f, 0.f, 0.f, static_cast<float>(Bottom));
	vertices.emplace_back(1.f, 0.f, 0.f, static_cast<float>(Bottom));
	vertices.emplace_back(1.f, 1.f, 0.f, static_cast<float>(Bottom));
	vertices.emplace_back(0.f, 1.f, 0.f, static_cast<float>(Bottom));
	AddQuad(indices, bottom, bottom + 3, bottom + 2, bottom + 1);

	m_Entity.GetComponent<MeshComponent>().mesh = VertexArray::CreateWithBuffers(vertices, indices,
		{ { ShaderDataType::Float4, "a_Position" } });
}

void MaterialBlock::SetSize(const std::array<float, 3>& size)
{
	// material X -> world X, material Y -> world -Z, heights are stored in mm and stay unscaled
	m_Entity.GetComponent<ScaleComponent>().scale = { size[0], 1.f, size[1] };
}
