#include "MaterialBlock.h"
#include <vector>
#include "scene/Scene.h"
#include "scene/Components.h"
#include "renderer/VertexArray.h"
#include <utility>

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

	std::vector<float> vertices;
	std::vector<uint32_t> indices;

	// top layer, one vertex per height map texel
	for (int j = 0; j < resolutionY; j++)
	{
		for (int i = 0; i < resolutionX; i++)
		{
			vertices.insert(vertices.end(), { stepU * i, stepV * j, 1, static_cast<float>(i), static_cast<float>(j) });
		}
	}
	
	//Bottom layer walls
	for (int i = 0; i < resolutionX; i++)
	{
		vertices.insert(vertices.end(), { stepU * i, 0, 0, static_cast<float>(-1), static_cast<float>(-1) });
		vertices.insert(vertices.end(), { stepU * i, 1, 0, static_cast<float>(-1), static_cast<float>(-1) });
	}

	for (int i = 0; i < resolutionY; i++)
	{
		vertices.insert(vertices.end(), { 0, stepV * i, 0, static_cast<float>(-1), static_cast<float>(-1) });
		vertices.insert(vertices.end(), { 1, stepV * i, 0, static_cast<float>(-1), static_cast<float>(-1) });
	}
	
	const uint32_t N = resolutionX * resolutionY;  // start of X-wall bottoms
	const uint32_t M = N + 2 * resolutionX;        // start of Y-wall bottoms

	// Top surface
	for (int j = 0; j < resolutionY - 1; j++)
	{
		for (int i = 0; i < resolutionX - 1; i++)
		{
			uint32_t a = j * resolutionX + i;
			uint32_t b = a + 1;
			uint32_t c = a + 1 + resolutionX;
			uint32_t d = a + resolutionX;
			indices.insert(indices.end(), { a, b, c, a, c, d });
		}
	}

	// Walls along X (y = 0 and y = 1)
	for (int i = 0; i < resolutionX - 1; i++)
	{
		uint32_t t0 = i, t1 = i + 1;                                                    // top, y = 0
		uint32_t u0 = (resolutionY - 1) * resolutionX + i, u1 = u0 + 1;                 // top, y = 1
		uint32_t b0 = N + 2 * i, b1 = N + 2 * (i + 1);                                  // bottom, y = 0
		indices.insert(indices.end(), { b0, b1, t1, b0, t1, t0 });                      // y = 0
		indices.insert(indices.end(), { b1 + 1, b0 + 1, u0, b1 + 1, u0, u1 });          // y = 1
	}

	// Walls along Y (x = 0 and x = 1)
	for (int j = 0; j < resolutionY - 1; j++)
	{
		uint32_t t0 = j * resolutionX, t1 = (j + 1) * resolutionX;                      // top, x = 0
		uint32_t u0 = t0 + resolutionX - 1, u1 = t1 + resolutionX - 1;                  // top, x = 1
		uint32_t b0 = M + 2 * j, b1 = M + 2 * (j + 1);                                  // bottom, x = 0
		indices.insert(indices.end(), { b1, b0, t0, b1, t0, t1 });                      // x = 0
		indices.insert(indices.end(), { b0 + 1, b1 + 1, u1, b0 + 1, u1, u0 });          // x = 1
	}

	// Bottom face
	uint32_t c00 = N, c01 = N + 1;
	uint32_t c10 = N + 2 * (resolutionX - 1), c11 = c10 + 1;
	indices.insert(indices.end(), { c00, c01, c11, c00, c11, c10 });

	m_Entity.GetComponent<MeshComponent>().mesh = VertexArray::CreateWithBuffers(vertices, indices,
		{ { ShaderDataType::Float3, "a_Position" }, { ShaderDataType::Float2, "a_Texel" } });
}

void MaterialBlock::SetSize(const std::array<float, 3>& size)
{
	m_Entity.GetComponent<ScaleComponent>().scale = { size[0], 1.f, size[1] };
}
