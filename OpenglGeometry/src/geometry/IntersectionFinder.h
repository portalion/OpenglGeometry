#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>
#include "Algebra.h"
#include "core/Base.h"
#include "scene/Entity.h"
#include "geometry/ParametricSurface.h"
#include "geometry/ParametricSurfaceFactory.h"

namespace Geometry
{
	struct IntersectionSettings
	{
		float stepLength = 0.05f;
		float precision = 1e-4f;
		bool useCursor = false;
		Algebra::Vector4 cursorPosition;
	};

	struct IntersectionData
	{
		std::vector<Algebra::Vector4> points;
		std::vector<Algebra::Vector4> paramsP;
		std::vector<Algebra::Vector4> paramsQ;
		bool closed = false;

		std::vector<uint32_t> componentEnds;
	};

	struct SurfaceParam
	{
		float uP = 0.f, vP = 0.f, uQ = 0.f, vQ = 0.f;

		SurfaceParam operator+(const SurfaceParam& o) const
		{
			return { uP + o.uP, vP + o.vP, uQ + o.uQ, vQ + o.vQ };
		}
		SurfaceParam operator*(float k) const
		{
			return { uP * k, vP * k, uQ * k, vQ * k };
		}
	};

	inline float Dot(const SurfaceParam& a, const SurfaceParam& b)
	{
		return a.uP * b.uP + a.vP * b.vP + a.uQ * b.uQ + a.vQ * b.vQ;
	}

	inline float IsSelfIntersectionTheSamePoint(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& x)
	{
		float du = x.uP - x.uQ;
		float dv = x.vP - x.vQ;
		if (p.WrappedU() || q.WrappedU())
		{
			du -= std::round(du);
		}
		if (p.WrappedV() || q.WrappedV())
		{
			dv -= std::round(dv);
		}
		return std::sqrt(du * du + dv * dv);
	}

	inline float GapSquared(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& x)
	{
		return LengthSq3(p.Evaluate(x.uP, x.vP) - q.Evaluate(x.uQ, x.vQ));
	}

	inline SurfaceParam GapGradient(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& x)
	{
		const Algebra::Vector4 diff = p.Evaluate(x.uP, x.vP) - q.Evaluate(x.uQ, x.vQ);
		return {
			 2.f * Dot3(diff, p.DerivativeU(x.uP, x.vP)),
			 2.f * Dot3(diff, p.DerivativeV(x.uP, x.vP)),
			-2.f * Dot3(diff, q.DerivativeU(x.uQ, x.vQ)),
			-2.f * Dot3(diff, q.DerivativeV(x.uQ, x.vQ)),
		};
	}

	inline SurfaceParam ClampParam(const IParametricSurface& p, const IParametricSurface& q,
		SurfaceParam x)
	{
		p.Clamp(x.uP, x.vP);
		q.Clamp(x.uQ, x.vQ);
		return x;
	}

	inline SurfaceParam MinimizeGapCG(const IParametricSurface& p, const IParametricSurface& q,
		SurfaceParam x, int maxIterations = 100)
	{
		x = ClampParam(p, q, x);

		SurfaceParam direction{};
		SurfaceParam previousGradient{};

		for (int iteration = 0; iteration < maxIterations; iteration++)
		{
			const SurfaceParam gradient = GapGradient(p, q, x);
			const float gradientNormSq = Dot(gradient, gradient);
			if (gradientNormSq < 1e-16f)
			{
				break;
			}

			if (iteration % 4 == 0)
			{
				direction = gradient * -1.f;
			}
			else
			{
				const float beta = gradientNormSq
					/ std::max(1e-20f, Dot(previousGradient, previousGradient));
				direction = gradient * -1.f + direction * beta;
				if (Dot(direction, gradient) > 0.f)
				{
					direction = gradient * -1.f;
				}
			}
			previousGradient = gradient;

			const float f0 = GapSquared(p, q, x);
			const float slope = Dot(gradient, direction);
			float step = 1.f;
			bool improved = false;
			for (int backtrack = 0; backtrack < 40; backtrack++)
			{
				const SurfaceParam trial = ClampParam(p, q, x + direction * step);
				if (GapSquared(p, q, trial) <= f0 + 1e-4f * step * slope)
				{
					x = trial;
					improved = true;
					break;
				}
				step *= 0.5f;
			}
			if (!improved)
			{
				break;
			}
		}

		return x;
	}

	inline bool SolveLinear4(std::array<std::array<float, 4>, 4> a, std::array<float, 4> b,
		std::array<float, 4>& x);
	inline Algebra::Vector4 IntersectionTangent(const IParametricSurface& p,
		const IParametricSurface& q, const SurfaceParam& x);
	inline bool NewtonCorrect(const IParametricSurface& p, const IParametricSurface& q,
		SurfaceParam& x, const Algebra::Vector4& anchor, const Algebra::Vector4& tangent,
		float arcStep);

	inline SurfaceParam PolishSelfSeed(const IParametricSurface& p, SurfaceParam x)
	{
		x = ClampParam(p, p, x);
		const Algebra::Vector4 anchor =
			(p.Evaluate(x.uP, x.vP) + p.Evaluate(x.uQ, x.vQ)) * 0.5f;
		const Algebra::Vector4 tangent = IntersectionTangent(p, p, x);

		SurfaceParam work = x;
		if (NewtonCorrect(p, p, work, anchor, tangent, 0.f))
		{
			return work;
		}
		return x;
	}

	inline SurfaceParam PolishSeed(const IParametricSurface& p, const IParametricSurface& q,
		SurfaceParam x)
	{
		x = ClampParam(p, q, x);

		for (int iteration = 0; iteration < 12; iteration++)
		{
			const Algebra::Vector4 r = p.Evaluate(x.uP, x.vP) - q.Evaluate(x.uQ, x.vQ);
			const float rNormSq = LengthSq3(r);
			if (rNormSq < 1e-18f)
			{
				break;
			}

			const Algebra::Vector4 col[4] = {
				p.DerivativeU(x.uP, x.vP), p.DerivativeV(x.uP, x.vP),
				q.DerivativeU(x.uQ, x.vQ) * -1.f, q.DerivativeV(x.uQ, x.vQ) * -1.f };

			std::array<std::array<float, 3>, 3> normal{};
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					float sum = 0.f;
					for (int k = 0; k < 4; k++)
					{
						sum += col[k][i] * col[k][j];
					}
					normal[i][j] = sum;
				}
			}

			std::array<float, 3> rhs{ r.x, r.y, r.z };
			std::array<float, 3> y{};
			bool ok = true;
			for (int c = 0; c < 3 && ok; c++)
			{
				int pivot = c;
				for (int rr = c + 1; rr < 3; rr++)
				{
					if (std::fabs(normal[rr][c]) > std::fabs(normal[pivot][c])) pivot = rr;
				}
				std::swap(normal[c], normal[pivot]);
				std::swap(rhs[c], rhs[pivot]);
				if (std::fabs(normal[c][c]) < 1e-14f) { ok = false; break; }
				for (int rr = c + 1; rr < 3; rr++)
				{
					const float fct = normal[rr][c] / normal[c][c];
					for (int k = c; k < 3; k++) normal[rr][k] -= fct * normal[c][k];
					rhs[rr] -= fct * rhs[c];
				}
			}
			if (!ok)
			{
				break;
			}
			for (int rr = 2; rr >= 0; rr--)
			{
				float sum = rhs[rr];
				for (int k = rr + 1; k < 3; k++) sum -= normal[rr][k] * y[k];
				y[rr] = sum / normal[rr][rr];
			}

			const SurfaceParam delta{
				col[0].x * y[0] + col[0].y * y[1] + col[0].z * y[2],
				col[1].x * y[0] + col[1].y * y[1] + col[1].z * y[2],
				col[2].x * y[0] + col[2].y * y[1] + col[2].z * y[2],
				col[3].x * y[0] + col[3].y * y[1] + col[3].z * y[2] };

			const SurfaceParam next = ClampParam(p, q, x + delta * -1.f);
			if (GapSquared(p, q, next) >= rNormSq)
			{
				break;
			}
			x = next;
		}

		return x;
	}

	inline std::vector<SurfaceParam> CollectSeeds(const IParametricSurface& p,
		const IParametricSurface& q, bool self, int grid,
		float tolerance)
	{
		const float minSeparation = self ? 0.08f : 0.f;

		std::vector<std::pair<float, SurfaceParam>> candidates;
		candidates.reserve((grid + 1) * (grid + 1));

		for (int a = 0; a <= grid; a++)
		{
			for (int b = 0; b <= grid; b++)
			{
				const float uP = static_cast<float>(a) / grid;
				const float vP = static_cast<float>(b) / grid;
				const Algebra::Vector4 pointP = p.Evaluate(uP, vP);

				float localGap = std::numeric_limits<float>::max();
				SurfaceParam localBest{};
				for (int c = 0; c <= grid; c++)
				{
					for (int d = 0; d <= grid; d++)
					{
						const float uQ = static_cast<float>(c) / grid;
						const float vQ = static_cast<float>(d) / grid;

						if (self && IsSelfIntersectionTheSamePoint(p, q, { uP, vP, uQ, vQ }) < minSeparation)
						{
							continue;
						}

						const float gap = LengthSq3(pointP - q.Evaluate(uQ, vQ));
						if (gap < localGap)
						{
							localGap = gap;
							localBest = { uP, vP, uQ, vQ };
						}
					}
				}
				if (localGap < std::numeric_limits<float>::max())
				{
					candidates.emplace_back(localGap, localBest);
				}
			}
		}

		std::sort(candidates.begin(), candidates.end(),
			[](const auto& l, const auto& r) { return l.first < r.first; });

		std::vector<SurfaceParam> seeds;
		int refined = 0;
		for (const auto& candidate : candidates)
		{
			if (refined >= 120 || seeds.size() >= 24)
			{
				break;
			}
			refined++;

			const SurfaceParam s = self
				? PolishSelfSeed(p, candidate.second)
				: PolishSeed(p, q, MinimizeGapCG(p, q, candidate.second));

			if (GapSquared(p, q, s) > tolerance)
			{
				continue;
			}
			if (self && IsSelfIntersectionTheSamePoint(p, q, s) < minSeparation)
			{
				continue;
			}

			const Algebra::Vector4 world = p.Evaluate(s.uP, s.vP);
			const bool haveIt = std::any_of(seeds.begin(), seeds.end(),
				[&](const SurfaceParam& t)
				{
					return LengthSq3(p.Evaluate(t.uP, t.vP) - world) < 1e-4f;
				});
			if (!haveIt)
			{
				seeds.push_back(s);
			}
		}

		return seeds;
	}
	
	inline bool SolveLinear4(std::array<std::array<float, 4>, 4> a, std::array<float, 4> b,
		std::array<float, 4>& x)
	{
		x = {};

		for (int col = 0; col < 4; col++)
		{
			int pivot = col;
			for (int row = col + 1; row < 4; row++)
			{
				if (std::fabs(a[row][col]) > std::fabs(a[pivot][col]))
				{
					pivot = row;
				}
			}
			std::swap(a[col], a[pivot]);
			std::swap(b[col], b[pivot]);

			if (std::fabs(a[col][col]) < 1e-12f)
			{
				return false;
			}

			for (int row = col + 1; row < 4; row++)
			{
				const float factor = a[row][col] / a[col][col];
				for (int k = col; k < 4; k++)
				{
					a[row][k] -= factor * a[col][k];
				}
				b[row] -= factor * b[col];
			}
		}

		for (int row = 3; row >= 0; row--)
		{
			float sum = b[row];
			for (int k = row + 1; k < 4; k++)
			{
				sum -= a[row][k] * x[k];
			}
			x[row] = sum / a[row][row];
		}
		return true;
	}

	inline Algebra::Vector4 IntersectionTangent(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& x)
	{
		Algebra::Vector4 tangent = Cross3(p.Normal(x.uP, x.vP), q.Normal(x.uQ, x.vQ));
		float length = tangent.Length();
		if (length < 1e-6f)
		{
			tangent = p.DerivativeU(x.uP, x.vP);
			length = tangent.Length();
		}
		if (length < 1e-9f)
		{
			return Algebra::Vector4(1.f, 0.f, 0.f, 0.f);
		}
		return tangent / length;
	}

	inline bool NewtonCorrect(const IParametricSurface& p, const IParametricSurface& q,
		SurfaceParam& x, const Algebra::Vector4& anchor, const Algebra::Vector4& tangent, float arcStep)
	{
		for (int iteration = 0; iteration < 20; iteration++)
		{
			const Algebra::Vector4 pointP = p.Evaluate(x.uP, x.vP);
			const Algebra::Vector4 pointQ = q.Evaluate(x.uQ, x.vQ);
			const Algebra::Vector4 midpoint = (pointP + pointQ) * 0.5f;
			const Algebra::Vector4 diff = pointP - pointQ;

			const std::array<float, 4> residual{
				diff.x, diff.y, diff.z, Dot3(midpoint - anchor, tangent) - arcStep };

			const Algebra::Vector4 pu = p.DerivativeU(x.uP, x.vP);
			const Algebra::Vector4 pv = p.DerivativeV(x.uP, x.vP);
			const Algebra::Vector4 qu = q.DerivativeU(x.uQ, x.vQ);
			const Algebra::Vector4 qv = q.DerivativeV(x.uQ, x.vQ);

			const std::array<std::array<float, 4>, 4> jacobian{ {
				{ pu.x, pv.x, -qu.x, -qv.x },
				{ pu.y, pv.y, -qu.y, -qv.y },
				{ pu.z, pv.z, -qu.z, -qv.z },
				{ 0.5f * Dot3(pu, tangent), 0.5f * Dot3(pv, tangent),
				  0.5f * Dot3(qu, tangent), 0.5f * Dot3(qv, tangent) },
			} };

			std::array<float, 4> delta{};
			if (!SolveLinear4(jacobian, residual, delta))
			{
				return false;
			}

			x.uP -= delta[0];
			x.vP -= delta[1];
			x.uQ -= delta[2];
			x.vQ -= delta[3];

			if (!std::isfinite(x.uP) || !std::isfinite(x.vP)
				|| !std::isfinite(x.uQ) || !std::isfinite(x.vQ))
			{
				return false;
			}

			const bool insideP = p.Clamp(x.uP, x.vP);
			const bool insideQ = q.Clamp(x.uQ, x.vQ);
			if (!insideP || !insideQ)
			{
				return false;
			}

			if (delta[0] * delta[0] + delta[1] * delta[1]
				+ delta[2] * delta[2] + delta[3] * delta[3] < 1e-12f)
			{
				return true;
			}
		}
		return false;
	}

	inline void MarchIntersection(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& seed, float arcStep, float gapTolerance, int maxSamples,
		std::vector<Algebra::Vector4>& points, std::vector<Algebra::Vector4>& paramsP,
		std::vector<Algebra::Vector4>& paramsQ, bool& closed)
	{
		SurfaceParam params = seed;
		SurfaceParam previous = seed;
		Algebra::Vector4 anchor =
			(p.Evaluate(params.uP, params.vP) + q.Evaluate(params.uQ, params.vQ)) * 0.5f;
		const Algebra::Vector4 startPoint = anchor;
		const float overshootSq = 9.f * arcStep * arcStep;
		Algebra::Vector4 lastTangent(0.f, 0.f, 0.f, 0.f);

		for (int sample = 0; sample < maxSamples; sample++)
		{
			Algebra::Vector4 tangent = IntersectionTangent(p, q, params);

			if (Dot3(tangent, lastTangent) < 0.f)
			{
				tangent = tangent * -1.f;
			}

			SurfaceParam attempt = params;
			bool ok = NewtonCorrect(p, q, attempt, anchor, tangent, arcStep);
			Algebra::Vector4 pointP = p.Evaluate(attempt.uP, attempt.vP);
			Algebra::Vector4 pointQ = q.Evaluate(attempt.uQ, attempt.vQ);
			Algebra::Vector4 candidate = (pointP + pointQ) * 0.5f;

			const auto badStep = [&]
			{
				return !ok
					|| LengthSq3(pointP - pointQ) > gapTolerance
					|| LengthSq3(candidate - anchor) > overshootSq;
			};

			for (int retry = 0; retry < 6 && badStep(); retry++)
			{
				attempt = previous;
				const float shorter = arcStep * std::pow(0.5f, static_cast<float>(retry + 1));
				ok = NewtonCorrect(p, q, attempt, anchor, tangent, shorter);
				pointP = p.Evaluate(attempt.uP, attempt.vP);
				pointQ = q.Evaluate(attempt.uQ, attempt.vQ);
				candidate = (pointP + pointQ) * 0.5f;
			}

			if (badStep())
			{
				return;
			}

			params = attempt;
			lastTangent = tangent;

			if (!points.empty() && LengthSq3(points.back() - candidate) < 1e-10f)
			{
				anchor = candidate;
				previous = params;
				continue;
			}

			if (sample > 6 && LengthSq3(candidate - startPoint) < arcStep * arcStep)
			{
				points.push_back(startPoint);
				paramsP.push_back(paramsP.empty()
					? Algebra::Vector4(seed.uP, seed.vP, 0.f, 0.f) : paramsP.front());
				paramsQ.push_back(paramsQ.empty()
					? Algebra::Vector4(seed.uQ, seed.vQ, 0.f, 0.f) : paramsQ.front());
				closed = true;
				return;
			}

			points.push_back(candidate);
			paramsP.push_back(Algebra::Vector4(params.uP, params.vP, 0.f, 0.f));
			paramsQ.push_back(Algebra::Vector4(params.uQ, params.vQ, 0.f, 0.f));

			anchor = candidate;
			previous = params;
		}
	}

	inline void TraceIntersection(const IParametricSurface& p, const IParametricSurface& q,
		const SurfaceParam& seed, float step, float gapTolerance, IntersectionData& out)
	{
		const int maxSamples = std::clamp(
			static_cast<int>(64.f / std::max(step, 1e-3f)) + 512, 512, 40000);

		std::vector<Algebra::Vector4> forwardPoints, forwardP, forwardQ;
		bool forwardClosed = false;
		MarchIntersection(p, q, seed, step, gapTolerance, maxSamples,
			forwardPoints, forwardP, forwardQ, forwardClosed);

		const Algebra::Vector4 seedPoint =
			(p.Evaluate(seed.uP, seed.vP) + q.Evaluate(seed.uQ, seed.vQ)) * 0.5f;
		const Algebra::Vector4 seedParamP(seed.uP, seed.vP, 0.f, 0.f);
		const Algebra::Vector4 seedParamQ(seed.uQ, seed.vQ, 0.f, 0.f);

		if (forwardClosed)
		{
			out.points.push_back(seedPoint);
			out.paramsP.push_back(seedParamP);
			out.paramsQ.push_back(seedParamQ);
			out.points.insert(out.points.end(), forwardPoints.begin(), forwardPoints.end());
			out.paramsP.insert(out.paramsP.end(), forwardP.begin(), forwardP.end());
			out.paramsQ.insert(out.paramsQ.end(), forwardQ.begin(), forwardQ.end());
			out.closed = true;
			return;
		}

		std::vector<Algebra::Vector4> backwardPoints, backwardP, backwardQ;
		bool backwardClosed = false;
		MarchIntersection(p, q, seed, -step, gapTolerance, maxSamples,
			backwardPoints, backwardP, backwardQ, backwardClosed);

		for (std::size_t i = backwardPoints.size(); i-- > 0; )
		{
			out.points.push_back(backwardPoints[i]);
			out.paramsP.push_back(backwardP[i]);
			out.paramsQ.push_back(backwardQ[i]);
		}
		out.points.push_back(seedPoint);
		out.paramsP.push_back(seedParamP);
		out.paramsQ.push_back(seedParamQ);
		out.points.insert(out.points.end(), forwardPoints.begin(), forwardPoints.end());
		out.paramsP.insert(out.paramsP.end(), forwardP.begin(), forwardP.end());
		out.paramsQ.insert(out.paramsQ.end(), forwardQ.begin(), forwardQ.end());
		out.closed = backwardClosed;
	}

	inline Algebra::Vector4 ProjectOntoSurface(const IParametricSurface& surface,
		const Algebra::Vector4& target, float guessU, float guessV, bool haveGuess)
	{
		float u = guessU;
		float v = guessV;

		if (!haveGuess)
		{
			constexpr int grid = 24;
			float best = std::numeric_limits<float>::max();
			for (int a = 0; a <= grid; a++)
			{
				for (int b = 0; b <= grid; b++)
				{
					const float su = static_cast<float>(a) / grid;
					const float sv = static_cast<float>(b) / grid;
					const float d = LengthSq3(surface.Evaluate(su, sv) - target);
					if (d < best)
					{
						best = d;
						u = su;
						v = sv;
					}
				}
			}
		}

		for (int iteration = 0; iteration < 40; iteration++)
		{
			const Algebra::Vector4 diff = surface.Evaluate(u, v) - target;
			const Algebra::Vector4 du = surface.DerivativeU(u, v);
			const Algebra::Vector4 dv = surface.DerivativeV(u, v);

			const float a = Dot3(du, du);
			const float b = Dot3(du, dv);
			const float d = Dot3(dv, dv);
			const float e = -Dot3(diff, du);
			const float f = -Dot3(diff, dv);

			const float det = a * d - b * b;
			if (std::fabs(det) < 1e-20f)
			{
				break;
			}

			const float deltaU = (e * d - b * f) / det;
			const float deltaV = (a * f - e * b) / det;

			u += deltaU;
			v += deltaV;
			surface.Clamp(u, v);

			if (deltaU * deltaU + deltaV * deltaV < 1e-14f)
			{
				break;
			}
		}

		return Algebra::Vector4(u, v, 0.f, 0.f);
	}

	inline void FillParametersFromPoints(IntersectionData& data,
		const IParametricSurface& surfaceP, const IParametricSurface& surfaceQ)
	{
		data.paramsP.clear();
		data.paramsQ.clear();
		data.paramsP.reserve(data.points.size());
		data.paramsQ.reserve(data.points.size());

		float pu = 0.5f, pv = 0.5f, qu = 0.5f, qv = 0.5f;
		bool have = false;

		for (const Algebra::Vector4& point : data.points)
		{
			const Algebra::Vector4 onP = ProjectOntoSurface(surfaceP, point, pu, pv, have);
			const Algebra::Vector4 onQ = ProjectOntoSurface(surfaceQ, point, qu, qv, have);

			pu = onP.x; pv = onP.y;
			qu = onQ.x; qv = onQ.y;
			have = true;

			data.paramsP.push_back(onP);
			data.paramsQ.push_back(onQ);
		}
	}

	inline IntersectionData FindIntersectionComponents(const IParametricSurface& surfaceP,
		const IParametricSurface& surfaceQ, bool self, const IntersectionSettings& settings)
	{
		IntersectionData result;

		const float step = std::clamp(settings.stepLength, 1e-3f, 0.5f);
		const float gapTolerance = std::max(settings.precision, 1e-7f);

		std::vector<SurfaceParam> seeds;

		if (settings.useCursor && !self)
		{
			const PointParametricSurface cursor(settings.cursorPosition);
			const SurfaceParam nearP = MinimizeGapCG(surfaceP, cursor, { 0.5f, 0.5f, 0.f, 0.f });
			const SurfaceParam nearQ = MinimizeGapCG(surfaceQ, cursor, { 0.5f, 0.5f, 0.f, 0.f });
			const SurfaceParam s = PolishSeed(surfaceP, surfaceQ,
				MinimizeGapCG(surfaceP, surfaceQ, { nearP.uP, nearP.vP, nearQ.uP, nearQ.vP }));
			if (GapSquared(surfaceP, surfaceQ, s) <= gapTolerance)
			{
				seeds.push_back(s);
			}
		}
		else
		{
			seeds = CollectSeeds(surfaceP, surfaceQ, self, 24, settings.precision);
		}

		const float mergeRadiusSq = 4.f * step * step;
		bool allClosed = true;

		for (const SurfaceParam& seed : seeds)
		{
			IntersectionData one;
			TraceIntersection(surfaceP, surfaceQ, seed, step, gapTolerance, one);
			if (one.points.size() < 2)
			{
				continue;
			}

			Algebra::Vector4 lo = one.points.front();
			Algebra::Vector4 hi = one.points.front();
			float length = 0.f;
			for (std::size_t i = 0; i < one.points.size(); i++)
			{
				lo.x = std::min(lo.x, one.points[i].x); hi.x = std::max(hi.x, one.points[i].x);
				lo.y = std::min(lo.y, one.points[i].y); hi.y = std::max(hi.y, one.points[i].y);
				lo.z = std::min(lo.z, one.points[i].z); hi.z = std::max(hi.z, one.points[i].z);
				if (i > 0)
				{
					length += (one.points[i] - one.points[i - 1]).Length();
				}
			}
			const float extent = (hi - lo).Length();
			if (extent > 1e-5f && length > 16.f * extent)
			{
				continue;
			}

			int matched = 0;
			for (const Algebra::Vector4& a : one.points)
			{
				for (const Algebra::Vector4& b : result.points)
				{
					if (LengthSq3(a - b) < mergeRadiusSq)
					{
						matched++;
						break;
					}
				}
			}
			if (!result.points.empty()
				&& matched * 10 >= static_cast<int>(one.points.size()) * 4)
			{
				continue;
			}

			result.points.insert(result.points.end(), one.points.begin(), one.points.end());
			result.paramsP.insert(result.paramsP.end(), one.paramsP.begin(), one.paramsP.end());
			result.paramsQ.insert(result.paramsQ.end(), one.paramsQ.begin(), one.paramsQ.end());
			result.componentEnds.push_back(static_cast<uint32_t>(result.points.size()));
			allClosed = allClosed && one.closed;
		}

		if (result.points.size() < 2)
		{
			return IntersectionData{};
		}
		result.closed = allClosed && !result.componentEnds.empty();
		return result;
	}

	inline IntersectionData FindIntersections(Entity entityP, Entity entityQ,
		const IntersectionSettings& settings)
	{
		const bool self = !entityQ.IsValid();

		Ref<IParametricSurface> surfaceP = MakeParametricSurface(entityP);
		Ref<IParametricSurface> surfaceQ = self ? surfaceP : MakeParametricSurface(entityQ);

		if (!surfaceP || !surfaceQ)
		{
			return IntersectionData{};
		}

		return FindIntersectionComponents(*surfaceP, *surfaceQ, self, settings);
	}
}
