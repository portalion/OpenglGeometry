#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include "Algebra.h"
#include "core/Base.h"

namespace Geometry
{
	inline float Dot3(const Algebra::Vector4& a, const Algebra::Vector4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline float LengthSq3(const Algebra::Vector4& a) { return Dot3(a, a); }

	inline Algebra::Vector4 Cross3(const Algebra::Vector4& a, const Algebra::Vector4& b)
	{
		return Algebra::Vector4(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
			0.f);
	}

	class IParametricSurface
	{
	public:
		virtual ~IParametricSurface() = default;

		virtual Algebra::Vector4 Evaluate(float u, float v) const = 0;
		virtual Algebra::Vector4 DerivativeU(float u, float v) const = 0;
		virtual Algebra::Vector4 DerivativeV(float u, float v) const = 0;

		virtual bool WrappedU() const = 0;
		virtual bool WrappedV() const = 0;

		Algebra::Vector4 Normal(float u, float v) const
		{
			Algebra::Vector4 n = Cross3(DerivativeU(u, v), DerivativeV(u, v));
			const float length = n.Length();
			if (length < 1e-9f)
			{
				return Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			}
			return n / length;
		}

		bool Fold(float& t, bool wrapped) const
		{
			if (t >= 0.f && t <= 1.f)
			{
				return true;
			}
			if (wrapped)
			{
				t -= std::floor(t);
				return true;
			}
			t = std::clamp(t, 0.f, 1.f);
			return false;
		}

		bool Clamp(float& u, float& v) const
		{
			const bool okU = Fold(u, WrappedU());
			const bool okV = Fold(v, WrappedV());
			return okU && okV;
		}
	};

	class PointParametricSurface : public IParametricSurface
	{
	public:
		explicit PointParametricSurface(const Algebra::Vector4& point) : m_Point(point)
		{
			m_Point.w = 1.f;
		}

		Algebra::Vector4 Evaluate(float, float) const override { return m_Point; }
		Algebra::Vector4 DerivativeU(float, float) const override { return Algebra::Vector4(0.f, 0.f, 0.f, 0.f); }
		Algebra::Vector4 DerivativeV(float, float) const override { return Algebra::Vector4(0.f, 0.f, 0.f, 0.f); }
		bool WrappedU() const override { return false; }
		bool WrappedV() const override { return false; }

	private:
		Algebra::Vector4 m_Point;
	};

	class TorusParametricSurface : public IParametricSurface
	{
	public:
		static constexpr float TwoPi = 6.28318530717958648f;

		TorusParametricSurface(float radius, float tubeRadius, const Algebra::Matrix4& model)
			: m_Radius(radius), m_TubeRadius(tubeRadius), m_Model(model)
		{
		}

		Algebra::Vector4 Evaluate(float u, float v) const override
		{
			const float theta = u * TwoPi;
			const float phi = v * TwoPi;
			const float ring = m_Radius + m_TubeRadius * std::cos(phi);

			const Algebra::Vector4 local(
				ring * std::cos(theta),
				m_TubeRadius * std::sin(phi),
				-ring * std::sin(theta),
				1.f);
			return m_Model * local;
		}

		Algebra::Vector4 DerivativeU(float u, float v) const override
		{
			const float theta = u * TwoPi;
			const float phi = v * TwoPi;
			const float ring = m_Radius + m_TubeRadius * std::cos(phi);

			const Algebra::Vector4 local(
				-ring * std::sin(theta) * TwoPi,
				0.f,
				-ring * std::cos(theta) * TwoPi,
				0.f);
			return m_Model * local;
		}

		Algebra::Vector4 DerivativeV(float u, float v) const override
		{
			const float theta = u * TwoPi;
			const float phi = v * TwoPi;
			const float dRing = -m_TubeRadius * std::sin(phi) * TwoPi;

			const Algebra::Vector4 local(
				dRing * std::cos(theta),
				m_TubeRadius * std::cos(phi) * TwoPi,
				-dRing * std::sin(theta),
				0.f);
			return m_Model * local;
		}

		bool WrappedU() const override { return true; }
		bool WrappedV() const override { return true; }

	private:
		float m_Radius;
		float m_TubeRadius;
		Algebra::Matrix4 m_Model;
	};

	class BezierSurfaceParametricSurface : public IParametricSurface
	{
	public:
		using Patch = std::array<std::array<Algebra::Vector4, 4>, 4>;

		BezierSurfaceParametricSurface(std::vector<std::vector<Patch>> patches)
			: m_Patches(std::move(patches))
		{
			m_Rows = static_cast<int>(m_Patches.size());
			m_Cols = m_Rows > 0 ? static_cast<int>(m_Patches[0].size()) : 0;
			m_WrappedU = DetectWrap(true);
			m_WrappedV = DetectWrap(false);
		}

		Algebra::Vector4 Evaluate(float u, float v) const override
		{
			int pi = 0, pj = 0;
			float lu = 0.f, lv = 0.f;
			Locate(u, m_Cols, pj, lu);
			Locate(v, m_Rows, pi, lv);

			const Patch& patch = m_Patches[pi][pj];
			std::array<Algebra::Vector4, 4> rows{};
			for (int i = 0; i < 4; i++)
			{
				rows[i] = DeCasteljau({ patch[i][0], patch[i][1], patch[i][2], patch[i][3] }, lu);
			}
			Algebra::Vector4 point = DeCasteljau(rows, lv);
			point.w = 1.f;
			return point;
		}

		Algebra::Vector4 DerivativeU(float u, float v) const override
		{
			int pi = 0, pj = 0;
			float lu = 0.f, lv = 0.f;
			Locate(u, m_Cols, pj, lu);
			Locate(v, m_Rows, pi, lv);

			const Patch& patch = m_Patches[pi][pj];
			std::array<Algebra::Vector4, 4> rows{};
			for (int i = 0; i < 4; i++)
			{
				rows[i] = CubicDerivative({ patch[i][0], patch[i][1], patch[i][2], patch[i][3] }, lu);
			}
			Algebra::Vector4 d = DeCasteljau(rows, lv) * static_cast<float>(m_Cols);
			d.w = 0.f;
			return d;
		}

		Algebra::Vector4 DerivativeV(float u, float v) const override
		{
			int pi = 0, pj = 0;
			float lu = 0.f, lv = 0.f;
			Locate(u, m_Cols, pj, lu);
			Locate(v, m_Rows, pi, lv);

			const Patch& patch = m_Patches[pi][pj];
			std::array<Algebra::Vector4, 4> rows{};
			for (int i = 0; i < 4; i++)
			{
				rows[i] = DeCasteljau({ patch[i][0], patch[i][1], patch[i][2], patch[i][3] }, lu);
			}
			Algebra::Vector4 d = CubicDerivative(rows, lv) * static_cast<float>(m_Rows);
			d.w = 0.f;
			return d;
		}

		bool WrappedU() const override { return m_WrappedU; }
		bool WrappedV() const override { return m_WrappedV; }

	private:
		static void Locate(float t, int segments, int& segment, float& local)
		{
			if (segments <= 1)
			{
				segment = 0;
				local = std::clamp(t, 0.f, 1.f);
				return;
			}
			const float scaled = std::clamp(t, 0.f, 1.f) * static_cast<float>(segments);
			segment = std::min(static_cast<int>(std::floor(scaled)), segments - 1);
			local = scaled - static_cast<float>(segment);
		}

		static Algebra::Vector4 DeCasteljau(const std::array<Algebra::Vector4, 4>& p, float t)
		{
			const float s = 1.f - t;
			const Algebra::Vector4 a = p[0] * s + p[1] * t;
			const Algebra::Vector4 b = p[1] * s + p[2] * t;
			const Algebra::Vector4 c = p[2] * s + p[3] * t;
			const Algebra::Vector4 d = a * s + b * t;
			const Algebra::Vector4 e = b * s + c * t;
			return d * s + e * t;
		}

		static Algebra::Vector4 CubicDerivative(const std::array<Algebra::Vector4, 4>& p, float t)
		{
			const float s = 1.f - t;
			return (p[1] - p[0]) * (3.f * s * s)
				+ (p[2] - p[1]) * (6.f * s * t)
				+ (p[3] - p[2]) * (3.f * t * t);
		}

		bool DetectWrap(bool alongU) const
		{
			if (m_Rows == 0 || m_Cols == 0)
			{
				return false;
			}
			for (float t : { 0.f, 0.2f, 0.45f, 0.7f, 0.95f })
			{
				const Algebra::Vector4 a = alongU ? Evaluate(0.f, t) : Evaluate(t, 0.f);
				const Algebra::Vector4 b = alongU ? Evaluate(1.f, t) : Evaluate(t, 1.f);
				if (LengthSq3(a - b) > 1e-6f)
				{
					return false;
				}
			}
			return true;
		}

		std::vector<std::vector<Patch>> m_Patches;
		int m_Rows = 0;
		int m_Cols = 0;
		bool m_WrappedU = false;
		bool m_WrappedV = false;
	};
}
