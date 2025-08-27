#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace Torus
	{
		Algebra::Vector4 GetPoint(float angleTube, float angleRadius, float radius, float tubeRadius);
		std::vector<Algebra::Vector4> GenerateVertices(
			float radius,
			float tubeRadius,
			unsigned int radialSegments,
			unsigned int tubularSegments
		);

		std::vector<uint32_t> GenerateIndices(
			unsigned int radialSegments,
			unsigned int tubularSegments
		);

		GeneratedMesh<Algebra::Vector4> GenerateMesh(
			float radius,
			float tubeRadius,
			unsigned int radialSegments = 16,
			unsigned int tubularSegments = 32
		);
	}
}

