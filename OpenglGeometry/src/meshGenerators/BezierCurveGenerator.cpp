#include "BezierCurveGenerator.h"

void MeshGenerator::BezierCurve::FixVertices(std::vector<Algebra::Vector4>& vertices)
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

std::vector<Algebra::Vector4> MeshGenerator::BezierCurve::GenerateVertices(const std::vector<Algebra::Vector4>& controlPoints)
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
