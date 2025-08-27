#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace Polyline
	{
		std::vector<uint32_t> GenerateIndices(unsigned int verticesSize);
		GeneratedMesh<Algebra::Vector4> GenerateMesh(
			const std::vector<Algebra::Vector4>& controlPoints
		);
	}
}
