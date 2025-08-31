#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace InterpolatedBezierCurve
	{
		std::vector<Algebra::Vector4> GenerateVertices(
			const std::vector<Algebra::Vector4>& controlPoints
		);
	}
}