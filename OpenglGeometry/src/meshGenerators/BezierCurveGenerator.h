#pragma once
#include "Base.h"

namespace MeshGenerator
{
	namespace BezierCurve
	{
		void FixVertices(std::vector<Algebra::Vector4>& vertices);
		std::vector<Algebra::Vector4> GenerateVertices(
			const std::vector<Algebra::Vector4>& controlPoints
		);
	}
}
