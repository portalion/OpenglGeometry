#include "BezierCurveGenerator.h"

#include <algorithm>

void MeshGenerator::BezierCurveC0::FixVertices(std::vector<Algebra::Vector4>& vertices)
{
	int fix = vertices.size() % 4;
	if (fix == 1) 
	{
		auto p = vertices[vertices.size() - 1];
		vertices.insert(vertices.end(), { p, p, p });
	}
	else if (fix == 2)
	{
		auto p0 = vertices[vertices.size() - 2];
		auto p1 = vertices[vertices.size() - 1];
		vertices.erase(vertices.end() - 2, vertices.end());
		vertices.insert(vertices.end(), { p0, p0, p1, p1 });
	}
	else if (fix == 3)
	{
		auto p0 = vertices[vertices.size() - 3];
		auto p1 = vertices[vertices.size() - 2];
		auto p2 = vertices[vertices.size() - 1];
		vertices.erase(vertices.end() - 3, vertices.end());
		vertices.insert(vertices.end(), 
		{ 
			p0, 
			(1.0f / 3.0f)* p0 + (2.0f / 3.0f) * p1,
			(2.0f / 3.0f)* p1 + (1.0f / 3.0f) * p2,
			p2 
		});
	}
}

std::vector<Algebra::Vector4> MeshGenerator::BezierCurveC0::GenerateVertices(const std::vector<Algebra::Vector4>& controlPoints)
{
	std::vector<Algebra::Vector4> result;

    if (controlPoints.empty())
    {
        return result;
    }

	result.push_back(controlPoints[0]);

    for (size_t i = 1; i < controlPoints.size(); i++)
    {
        result.push_back(controlPoints[i]);
        if (i % 3 == 0)
        {
            result.push_back(controlPoints[i]);
        }
    }

	FixVertices(result);

    return result;
}

std::vector<Algebra::Vector4> MeshGenerator::BezierCurveC2::GenerateVertices(const std::vector<Algebra::Vector4>& bSplineControlPoints)
{
	if (bSplineControlPoints.size() < 4)
	{
		return { };
	}

	std::vector<Algebra::Vector4> result;
	result.reserve(bSplineControlPoints.size() * 4);

	for (size_t i = 0; i + 3 < bSplineControlPoints.size(); i++)
	{
		const Algebra::Vector4& P0 = bSplineControlPoints[i];
		const Algebra::Vector4& P1 = bSplineControlPoints[i + 1];
		const Algebra::Vector4& P2 = bSplineControlPoints[i + 2];
		const Algebra::Vector4& P3 = bSplineControlPoints[i + 3];

		Algebra::Vector4 B0 = (P0 + 4.0f * P1 + P2) / 6.0f;
		Algebra::Vector4 B1 = (4.0f * P1 + 2.0f * P2) / 6.0f;
		Algebra::Vector4 B2 = (2.0f * P1 + 4.0f * P2) / 6.0f;
		Algebra::Vector4 B3 = (P1 + 4.0f * P2 + P3) / 6.0f;

		result.insert(result.end(), { B0, B1, B2, B3 });
	}

	return result;
}

std::vector<Algebra::Vector4> MeshGenerator::BezierCurveC2::ToBernsteinPoints(
	const std::vector<Algebra::Vector4>& deBoorPoints)
{
	std::vector<Algebra::Vector4> result;

	if (deBoorPoints.size() < 4)
	{
		return result;
	}

	for (size_t i = 0; i + 3 < deBoorPoints.size(); i++)
	{
		const Algebra::Vector4& P0 = deBoorPoints[i];
		const Algebra::Vector4& P1 = deBoorPoints[i + 1];
		const Algebra::Vector4& P2 = deBoorPoints[i + 2];
		const Algebra::Vector4& P3 = deBoorPoints[i + 3];

		result.push_back((P0 + 4.0f * P1 + P2) / 6.0f);
		result.push_back((4.0f * P1 + 2.0f * P2) / 6.0f);
		result.push_back((2.0f * P1 + 4.0f * P2) / 6.0f);

		if (i + 4 >= deBoorPoints.size())
		{
			result.push_back((P1 + 4.0f * P2 + P3) / 6.0f);
		}
	}

	return result;
}

size_t MeshGenerator::BezierCurveC2::DeBoorIndexOf(size_t bernsteinIndex, size_t deBoorCount)
{
	const size_t segment = bernsteinIndex / 3;
	const size_t withinSegment = bernsteinIndex % 3;

	const size_t index = withinSegment == 2 ? segment + 2 : segment + 1;

	return deBoorCount == 0 ? 0 : std::min(index, deBoorCount - 1);
}
