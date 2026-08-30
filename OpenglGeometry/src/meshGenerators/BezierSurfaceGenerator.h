#pragma once
#include <array>
#include "Base.h"

namespace MeshGenerator
{
	namespace BezierSurfaceC2
	{
		using PatchGrid = std::array<std::array<Algebra::Vector4, 4>, 4>;

		inline PatchGrid DeBoorToBernstein(const PatchGrid& deBoor)
		{
			static const float A[4][4] = {
				{ 1.f / 6.f, 4.f / 6.f, 1.f / 6.f, 0.f },
				{ 0.f,       4.f / 6.f, 2.f / 6.f, 0.f },
				{ 0.f,       2.f / 6.f, 4.f / 6.f, 0.f },
				{ 0.f,       1.f / 6.f, 4.f / 6.f, 1.f / 6.f },
			};

			PatchGrid intermediate{};
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
				{
					Algebra::Vector4 sum;
					for (int k = 0; k < 4; k++)
						sum = sum + A[i][k] * deBoor[k][j];
					intermediate[i][j] = sum;
				}

			PatchGrid bernstein{};
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
				{
					Algebra::Vector4 sum;
					for (int k = 0; k < 4; k++)
						sum = sum + intermediate[i][k] * A[j][k];
					bernstein[i][j] = sum;
				}

			return bernstein;
		}
	}
}
