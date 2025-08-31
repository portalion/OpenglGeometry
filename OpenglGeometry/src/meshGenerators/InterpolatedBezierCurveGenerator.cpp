#include "InterpolatedBezierCurveGenerator.h"

std::vector<Algebra::Vector4> MeshGenerator::InterpolatedBezierCurve::
	GenerateVertices(const std::vector<Algebra::Vector4>& controlPoints)
{
	if (controlPoints.size() < 2)
		return { };

	if (controlPoints.size() == 2)
	{
		return { controlPoints[0], controlPoints[0], controlPoints[1], controlPoints[1] };
	}

	std::vector<Algebra::Vector4> result;
	std::vector<float> d;
	std::vector<float> alpha;
	std::vector<float> beta;
	std::vector<Algebra::Vector4> r;
	for (int i = 0; i < controlPoints.size() - 1; i++)
	{
		d.push_back((controlPoints[i + 1] - controlPoints[i]).Length());
	}
	for (int i = 1; i < d.size(); i++)
	{
		float d0 = d[i - 1];
		float d1 = d[i];
		if (i != 1)
			alpha.push_back(d0 / (d0 + d1));
		if (i != d.size() - 1)
			beta.push_back(d1 / (d0 + d1));

		Algebra::Vector4 P0 = (controlPoints[i] - controlPoints[i - 1]) / d0;
		Algebra::Vector4 P1 = (controlPoints[i + 1] - controlPoints[i]) / d1;

		r.push_back(3.f * (P1 - P0) / (d0 + d1));
	}
	for (int i = 0; i < alpha.size(); i++)
	{
		if (std::isnan(alpha[i])) alpha[i] = 0.f;
		if (std::isnan(beta[i])) beta[i] = 0.f;
	}
	auto c = Algebra::Matrix4::SolveTrilinealEquation(alpha, beta, r);
	c.insert(c.begin(), Algebra::Vector4());
	c.push_back(Algebra::Vector4());
	std::vector<Algebra::Vector4> a(c.size());
	std::vector<Algebra::Vector4> b(c.size());
	std::vector<Algebra::Vector4> D(c.size());

	for (int i = 0; i < d.size(); i++)
		D[i] = (c[i + 1] - c[i]) / d[i] / 3.f;

	for (int i = 0; i < a.size(); i++)
		a[i] = controlPoints[i];

	for (int i = 0; i < b.size() - 1; i++)
		b[i] = (a[i + 1] - a[i]) / d[i] - c[i] * d[i] - D[i] * d[i] * d[i];
	for (size_t i = 0; i < d.size(); ++i)
	{
		Algebra::Vector4 ai = a[i];
		Algebra::Vector4 bi = b[i] * d[i];
		Algebra::Vector4 ci = c[i] * d[i] * d[i];
		Algebra::Vector4 di = D[i] * d[i] * d[i] * d[i];

		Algebra::Vector4 P0 = ai;
		Algebra::Vector4 P1 = ai + bi / 3.f;
		Algebra::Vector4 P2 = ai + bi * 2.f / 3.f + ci / 3.f;
		Algebra::Vector4 P3 = ai + bi + ci + di;
		P0.w = P1.w = P2.w = P3.w = 1.0f;

		result.insert(result.end(), { P0, P1, P2, P3 });
	}

	return result;
}
