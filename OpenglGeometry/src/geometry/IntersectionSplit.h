#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
#include "Algebra.h"
#include "core/Base.h"
#include "geometry/IntersectionFinder.h"

namespace Geometry
{
	namespace Detail
	{
		inline float MedianSegmentLength(const std::vector<Algebra::Vector4>& pts)
		{
			if (pts.size() < 2) return 0.f;
			std::vector<float> lens;
			lens.reserve(pts.size());
			for (std::size_t i = 1; i < pts.size(); i++)
			{
				lens.push_back((pts[i] - pts[i - 1]).Length());
			}
			std::sort(lens.begin(), lens.end());
			return lens[lens.size() / 2];
		}

		inline std::vector<Algebra::Vector4> SliceRing(const std::vector<Algebra::Vector4>& src,
			std::size_t a, std::size_t b, std::size_t n)
		{
			std::vector<Algebra::Vector4> out;
			std::size_t i = a;
			while (true)
			{
				out.push_back(src[i]);
				if (i == b) break;
				i = (i + 1) % n;
			}
			out.push_back(src[a]);
			return out;
		}
	}

	inline void SplitSelfCrossingLoop(const std::vector<Algebra::Vector4>& points,
		const std::vector<Algebra::Vector4>& paramsP,
		const std::vector<Algebra::Vector4>& paramsQ,
		std::vector<IntersectionData>& out)
	{
		const auto emitWhole = [&]
		{
			IntersectionData piece;
			piece.points = points;
			piece.paramsP = paramsP;
			piece.paramsQ = paramsQ;
			piece.closed = true;
			out.push_back(std::move(piece));
		};

		std::size_t n = points.size();
		if (n >= 2 && (points[n - 1] - points[0]).Length() < 1e-5f)
		{
			n--;
		}

		const std::size_t minArc = std::max<std::size_t>(6, n / 8);
		if (n < 2 * minArc + 2)
		{
			emitWhole();
			return;
		}

		const float seg = Detail::MedianSegmentLength(points);
		const float threshold = std::max(1e-6f, 1.5f * seg);

		float best = std::numeric_limits<float>::max();
		std::size_t bi = 0, bj = 0;
		for (std::size_t i = 0; i < n; i++)
		{
			for (std::size_t j = i + minArc; j < n; j++)
			{
				if (n - (j - i) < minArc) continue;

				const float d = (points[i] - points[j]).Length();
				if (d < best)
				{
					best = d;
					bi = i;
					bj = j;
				}
			}
		}

		if (best > threshold)
		{
			emitWhole();
			return;
		}

		const bool haveP = paramsP.size() >= n;
		const bool haveQ = paramsQ.size() >= n;
		const auto slice = [&](const std::vector<Algebra::Vector4>& src, std::size_t a,
			std::size_t b, bool have) -> std::vector<Algebra::Vector4>
		{
			return have ? Detail::SliceRing(src, a, b, n) : std::vector<Algebra::Vector4>{};
		};

		IntersectionData a;
		a.points = Detail::SliceRing(points, bi, bj, n);
		a.paramsP = slice(paramsP, bi, bj, haveP);
		a.paramsQ = slice(paramsQ, bi, bj, haveQ);
		a.closed = true;
		out.push_back(std::move(a));

		IntersectionData b;
		b.points = Detail::SliceRing(points, bj, bi, n);
		b.paramsP = slice(paramsP, bj, bi, haveP);
		b.paramsQ = slice(paramsQ, bj, bi, haveQ);
		b.closed = true;
		out.push_back(std::move(b));
	}

	inline std::vector<IntersectionData> SplitSelfCrossings(const IntersectionData& data)
	{
		std::vector<uint32_t> ends = data.componentEnds;
		if (ends.empty() || ends.back() != data.points.size())
		{
			ends.push_back(static_cast<uint32_t>(data.points.size()));
		}

		std::vector<IntersectionData> pieces;
		uint32_t start = 0;
		for (uint32_t end : ends)
		{
			if (end <= start) continue;
			const auto sliceP = data.paramsP.size() >= end
				? std::vector<Algebra::Vector4>(data.paramsP.begin() + start, data.paramsP.begin() + end)
				: std::vector<Algebra::Vector4>{};
			const auto sliceQ = data.paramsQ.size() >= end
				? std::vector<Algebra::Vector4>(data.paramsQ.begin() + start, data.paramsQ.begin() + end)
				: std::vector<Algebra::Vector4>{};
			SplitSelfCrossingLoop(
				std::vector<Algebra::Vector4>(data.points.begin() + start, data.points.begin() + end),
				sliceP, sliceQ, pieces);
			start = end;
		}

		if (pieces.size() <= 1)
		{
			return { data };
		}
		return pieces;
	}
}
