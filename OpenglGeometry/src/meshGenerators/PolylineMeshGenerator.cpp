#include "PolylineMeshGenerator.h"

std::vector<uint32_t> MeshGenerator::Polyline::GenerateIndices(unsigned int verticesSize)
{
    std::vector<uint32_t> indices;

    for (int i = 0; i < static_cast<int>(verticesSize) - 1; i++)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    return indices;
}

MeshGenerator::GeneratedMesh<Algebra::Vector4> MeshGenerator::Polyline::
    GenerateMesh(std::vector<Algebra::Vector4> controlPoints)
{
    GeneratedMesh<Algebra::Vector4> result;

    result.vertices = controlPoints;
    result.indices = GenerateIndices(static_cast<unsigned int>(controlPoints.size()));

    return result;
}
