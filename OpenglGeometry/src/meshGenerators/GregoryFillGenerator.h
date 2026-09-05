#pragma once
#include <array>
#include <cmath>
#include "Algebra.h"

namespace MeshGenerator
{
	namespace GregoryFill
	{
		using Point = Algebra::Vector4;
		using CubicBezierControl = std::array<Point, 4>;
		using GregoryControlPoints = std::array<Point, 20>;

		struct HoleBoundarySide
		{
			CubicBezierControl curve;
			CubicBezierControl inner;
		};

		struct HalfSplit
		{
			Point r0;
			Point r2;
			Point s0;
			Point s1;
			Point mid;
		};

		inline Point Mid(const Point& a, const Point& b)
		{
			return (a + b) / 2.0f;
		}

		inline HalfSplit SubdivideAtHalf(const CubicBezierControl& c)
		{
			const Point a0 = Mid(c[0], c[1]);
			const Point a1 = Mid(c[1], c[2]);
			const Point a2 = Mid(c[2], c[3]);
			const Point b0 = Mid(a0, a1);
			const Point b1 = Mid(a1, a2);
			const Point m = Mid(b0, b1);

			return HalfSplit{ a0, a2, b0, b1, m };
		}

		struct EdgeData
		{
			HalfSplit boundary;
			HalfSplit inner;
			Point corner;
			Point p2;
		};

		inline Point Blend(const Point& start, const Point& head, const Point& tail)
		{
			return start + (head - tail);
		}

		namespace NetIndex
		{
			constexpr int P00 = 0, P01 = 3, P10 = 16, P11 = 19;

			constexpr int VuvAt00 = 6, VvuAt00 = 5;
			constexpr int VuvAt01 = 7, VvuAt01 = 8;
			constexpr int VuvAt10 = 12, VvuAt10 = 11;
			constexpr int VuvAt11 = 13, VvuAt11 = 14;
		}

		inline std::array<GregoryControlPoints, 3> BuildTriangular(const std::array<HoleBoundarySide, 3>& sides)
		{
			using namespace NetIndex;

			std::array<EdgeData, 3> edge{};

			for (int k = 0; k < 3; k++)
			{
				edge[k].boundary = SubdivideAtHalf(sides[k].curve);
				edge[k].inner = SubdivideAtHalf(sides[k].inner);
				edge[k].corner = sides[k].curve[3];
				edge[k].p2 = 2.0f * edge[k].boundary.mid - edge[k].inner.mid;
			}
			const Point center = (edge[0].p2 + edge[1].p2 + edge[2].p2) / 3.0f;
			std::array<Point, 3> spoke{};
			for (int k = 0; k < 3; k++)
			{
				spoke[k] = (2.0f * edge[k].p2 + center) / 3.0f;
			}

			std::array<GregoryControlPoints, 3> nets{};

			for (int k = 0; k < 3; k++)
			{
				const int n = (k + 1) % 3;
				const HalfSplit& be = edge[k].boundary;
				const HalfSplit& ie = edge[k].inner;
				const HalfSplit& bn = edge[n].boundary;
				const HalfSplit& in = edge[n].inner;

				GregoryControlPoints& g = nets[k];

				g[P00] = be.mid;
				g[1] = edge[k].p2;
				g[2] = spoke[k];
				g[P01] = center;

				g[4] = be.s1;
				g[10] = be.r2;
				g[P10] = edge[k].corner;

				g[9] = spoke[n];
				g[15] = edge[n].p2;
				g[P11] = bn.mid;

				g[17] = bn.r0;
				g[18] = bn.s0;

				g[VvuAt00] = 2.0f * be.s1 - ie.s1;
				g[VuvAt00] = g[VvuAt00];
				g[VvuAt10] = 2.0f * be.r2 - ie.r2;
				g[VuvAt11] = 2.0f * bn.s0 - in.s0;
				g[VvuAt11] = g[VuvAt11];
				g[VuvAt10] = 2.0f * bn.r0 - in.r0;
				g[VuvAt01] = Blend(g[VvuAt00], g[2], g[1]);
				g[VvuAt01] = Blend(g[VuvAt11], g[9], g[15]);

				for (Point& point : g)
				{
					point.w = 1.0f;
				}
			}

			return nets;
		}
	}
}
