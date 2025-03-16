#include "Torus.h"

Algebra::Vector4 Torus::GetPoint(float angleTube, float angleRadius)
{
	return Algebra::Matrix4::RotationY(angleRadius) * 
		Algebra::Vector4(radius + tubeRadius * cosf(angleTube), tubeRadius * sinf(angleTube), 0.f, 1.f);
}

void Torus::GenerateAndSaveMesh()
{
	std::vector<PositionVertexData> vertices;
	std::vector<unsigned int> indices;

	for (int i = 0; i < radiusSegments; i++)
	{
		for (int j = 0; j < tubeSegments; j++)
		{
			PositionVertexData vertex;
			vertex.Position = GetPoint(2 * 3.14f * j / tubeSegments, 2 * 3.14f * i / radiusSegments);
			vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < radiusSegments; i++)
	{
		int iNext = (i + 1) % radiusSegments;
		for (int j = 0; j < tubeSegments; j++)
		{
			int jNext = (j + 1) % tubeSegments;
			indices.push_back(tubeSegments * i + j);
			indices.push_back(tubeSegments * i + jNext);
			indices.push_back(tubeSegments * i + j);
			indices.push_back((tubeSegments * iNext) + j);
		}
	}

	renderer.AssignVertices(vertices);
	renderer.AssignIndices(indices);
}

Torus::Torus()
	:renderer{ VertexDataType::PositionVertexData }
{
	GenerateAndSaveMesh();
}

void Torus::Render()
{
	renderer.Render(GL_LINES);
}
