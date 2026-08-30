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

	namespace BezierCurveC2
	{
		std::vector<Algebra::Vector4> GenerateVertices(
			const std::vector<Algebra::Vector4>& bSplineControlPoints
		);

		std::vector<Algebra::Vector4> ToBernsteinPoints(
			const std::vector<Algebra::Vector4>& deBoorPoints
		);

		size_t DeBoorIndexOf(size_t bernsteinIndex, size_t deBoorCount);
	}
}
