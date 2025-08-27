#pragma once
#include <vector>
#include "Algebra.h"
#include "renderer/VertexBuffer.h"
#include "renderer/Renderer.h"
#include "managers/ShaderManager.h"

namespace MeshGenerator
{
	template <typename VertexType>
	struct GeneratedMesh
	{
		std::vector<VertexType> vertices;
		std::vector<uint32_t> indices;
		AvailableShaders shaderType = AvailableShaders::Default;
		RenderingMode renderingMode = RenderingMode::Lines;
		BufferLayout layout = 
		{
			{ ShaderDataType::Float4, "position" }
		};
	};
}
