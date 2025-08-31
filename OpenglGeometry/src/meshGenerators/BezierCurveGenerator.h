#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace BezierCurveC0
	{
		void FixVertices(std::vector<Algebra::Vector4>& vertices);
		std::vector<Algebra::Vector4> GenerateVertices(
			const std::vector<Algebra::Vector4>& controlPoints
		);
	}
}
