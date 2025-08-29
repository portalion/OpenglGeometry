#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace InterpolatedBezierCurve
	{
		GeneratedMesh<Algebra::Vector4> GenerateMesh(
			const std::vector<Algebra::Vector4>& controlPoints
		);
	}
}