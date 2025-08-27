#include "TorusMeshGenerator.h"

Algebra::Vector4 MeshGenerator::Torus::
	GetPoint(float angleTube, float angleRadius, float radius, float tubeRadius)
{
	return Algebra::Matrix4::RotationY(angleRadius) *
		Algebra::Vector4(radius + tubeRadius * cosf(angleTube), tubeRadius * sinf(angleTube), 0.f, 1.f);
}

std::vector<Algebra::Vector4> MeshGenerator::Torus::GenerateVertices(float radius, float tubeRadius, unsigned int radialSegments, unsigned int tubularSegments)
{
	std::vector<Algebra::Vector4> vertices;

	for (unsigned int i = 0; i < radialSegments; i++)
	{
		for (unsigned int j = 0; j < tubularSegments; j++)
		{
			const auto& point = GetPoint(
				2 * 3.14f * j / tubularSegments, 2 * 3.14f * i / radialSegments, radius, tubeRadius);
			vertices.push_back(point);
		}
	}
	return vertices;
}

std::vector<uint32_t> MeshGenerator::Torus::GenerateIndices(unsigned int radialSegments, unsigned int tubularSegments)
{
	std::vector<uint32_t> indices;

	for (unsigned int i = 0; i < radialSegments; i++)
	{
		int iNext = (i + 1) % radialSegments;
		for (unsigned int j = 0; j < tubularSegments; j++)
		{
			int jNext = (j + 1) % tubularSegments;
			indices.push_back(tubularSegments * i + j);
			indices.push_back(tubularSegments * i + jNext);
			indices.push_back(tubularSegments * i + j);
			indices.push_back((tubularSegments * iNext) + j);
		}
	}

	return indices;
}

MeshGenerator::GeneratedMesh<Algebra::Vector4> MeshGenerator::Torus::
	GenerateMesh(float radius, float tubeRadius, unsigned int radialSegments, unsigned int tubularSegments)
{
	GeneratedMesh<Algebra::Vector4> result;

	result.vertices = GenerateVertices(radius, tubeRadius, radialSegments, tubularSegments);
	result.indices = GenerateIndices(radialSegments, tubularSegments);
	
	return result;
}
