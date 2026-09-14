#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <queue>
#include <vector>
#include "Algebra.h"

namespace Geometry
{
	struct TrimMaskData
	{
		int width = 0;
		int height = 0;
		std::vector<uint8_t> pixels;
	};

	namespace Detail
	{
		inline void FillLineSegment(std::vector<uint8_t>& line, int res,
			float u0, float v0, float u1, float v1)
		{
			const int steps = std::max(1, static_cast<int>(
				std::ceil(std::max(std::fabs(u1 - u0), std::fabs(v1 - v0)) * res * 2.f)));
			for (int i = 0; i <= steps; i++)
			{
				const float t = static_cast<float>(i) / steps;
				int x = static_cast<int>(std::floor((u0 + (u1 - u0) * t) * res));
				int y = static_cast<int>(std::floor((v0 + (v1 - v0) * t) * res));
				x = std::clamp(x, 0, res - 1);
				y = std::clamp(y, 0, res - 1);
				line[y * res + x] = 1;
				if (x + 1 < res) line[y * res + x + 1] = 1;
				if (y + 1 < res) line[(y + 1) * res + x] = 1;
			}
		}

		inline float Wrap01(float t)
		{
			t -= std::floor(t);
			return t;
		}
	}

	inline void FillLine(const std::vector<Algebra::Vector4>& params, bool wrapU, bool wrapV, int res, std::vector<uint8_t>& line)
	{
		for (std::size_t i = 1; i < params.size(); i++)
		{
			float u0 = params[i - 1].x, v0 = params[i - 1].y;
			float u1 = params[i].x, v1 = params[i].y;

			if (wrapU && std::fabs(u1 - u0) > 0.5f) u1 += (u0 > u1 ? 1.f : -1.f);
			if (wrapV && std::fabs(v1 - v0) > 0.5f) v1 += (v0 > v1 ? 1.f : -1.f);

			Detail::FillLineSegment(line, res, u0, v0, u1, v1);
			if (u1 < 0.f || u1 > 1.f || v1 < 0.f || v1 > 1.f)
			{
				const float du = (u1 < 0.f) ? 1.f : (u1 > 1.f ? -1.f : 0.f);
				const float dv = (v1 < 0.f) ? 1.f : (v1 > 1.f ? -1.f : 0.f);
				Detail::FillLineSegment(line, res, u0 + du, v0 + dv, u1 + du, v1 + dv);
			}
		}
	}

	inline TrimMaskData FloodFill(const std::vector<Algebra::Vector4>& params,
		bool wrapU, bool wrapV, int res)
	{
		res = std::max(16, res);
		const std::size_t cells = static_cast<std::size_t>(res) * res;
		std::vector<uint8_t> wall(cells, 0);

		FillLine(params, wrapU, wrapV, res, wall);

		const float baseU = params[0].x, baseV = params[0].y;
		float sumU = 0.f, sumV = 0.f;
		for (const auto& p : params)
		{
			float du = p.x - baseU, dv = p.y - baseV;
			if (wrapU) du -= std::round(du);
			if (wrapV) dv -= std::round(dv);
			sumU += baseU + du;
			sumV += baseV + dv;
		}
		const float centroidU = sumU / params.size();
		const float centroidV = sumV / params.size();

		float aU = params[0].x, aV = params[0].y;
		float bU = params[1].x, bV = params[1].y;
		if (wrapU && std::fabs(bU - aU) > 0.5f) bU += (aU > bU ? 1.f : -1.f);
		if (wrapV && std::fabs(bV - aV) > 0.5f) bV += (aV > bV ? 1.f : -1.f);
		const float midU = 0.5f * (aU + bU);
		const float midV = 0.5f * (aV + bV);

		float dirU = -(bV - aV);
		float dirV = (bU - aU);
		float dirLen = std::sqrt(dirU * dirU + dirV * dirV);
		if (dirLen < 1e-9f)
		{
			dirU = centroidU - midU;
			dirV = centroidV - midV;
			dirLen = std::max(1e-9f, std::sqrt(dirU * dirU + dirV * dirV));
		}
		dirU /= dirLen;
		dirV /= dirLen;
		if (dirU * (centroidU - midU) + dirV * (centroidV - midV) < 0.f)
		{
			dirU = -dirU;
			dirV = -dirV;
		}

		const auto cellOf = [&](float u, float v, int& x, int& y)
		{
			u = wrapU ? Detail::Wrap01(u) : std::clamp(u, 0.f, 1.f);
			v = wrapV ? Detail::Wrap01(v) : std::clamp(v, 0.f, 1.f);
			x = std::min(res - 1, static_cast<int>(u * res));
			y = std::min(res - 1, static_cast<int>(v * res));
		};

		int seedIdx = -1;
		for (float sign : { 1.f, -1.f })
		{
			for (int k = 2; k <= 12 && seedIdx < 0; k++)
			{
				const float step = sign * static_cast<float>(k) * 1.5f / res;
				int x = 0, y = 0;
				cellOf(midU + dirU * step, midV + dirV * step, x, y);
				if (!wall[y * res + x])
				{
					seedIdx = y * res + x;
				}
			}
			if (seedIdx >= 0) break;
		}
		if (seedIdx < 0)
		{
			// fall back to the centroid cell
			int x = 0, y = 0;
			cellOf(centroidU, centroidV, x, y);
			seedIdx = y * res + x;
		}

		//Start Flood-fill
		std::vector<uint8_t> filled(cells, 0);
		std::queue<int> current;
		if (!wall[seedIdx])
		{
			filled[seedIdx] = 1;
			current.push(seedIdx);
		}
		while (!current.empty())
		{
			const int idx = current.front();
			current.pop();
			const int x = idx % res;
			const int y = idx / res;

			const auto visit = [&](int nx, int ny)
			{
				if (nx < 0) { if (!wrapU) return; nx = res - 1; }
				if (nx >= res) { if (!wrapU) return; nx = 0; }
				if (ny < 0) { if (!wrapV) return; ny = res - 1; }
				if (ny >= res) { if (!wrapV) return; ny = 0; }
				const int n = ny * res + nx;
				if (!wall[n] && !filled[n])
				{
					filled[n] = 1;
					current.push(n);
				}
			};
			visit(x - 1, y);
			visit(x + 1, y);
			visit(x, y - 1);
			visit(x, y + 1);
		}

		TrimMaskData mask;
		mask.width = res;
		mask.height = res;
		mask.pixels.resize(cells * 4);
		for (std::size_t i = 0; i < cells; i++)
		{
			const uint8_t value = (filled[i] || wall[i]) ? 255 : 0;
			mask.pixels[i * 4 + 0] = value;
			mask.pixels[i * 4 + 1] = value;
			mask.pixels[i * 4 + 2] = value;
			mask.pixels[i * 4 + 3] = 255;
		}
		return mask;
	}
}
