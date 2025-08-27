#include "BezierCurveGenerator.h"

void MeshGenerator::BezierCurve::FixMesh(GeneratedMesh<Algebra::Vector4>& mesh)
{
    unsigned int meshLastBlockSize = mesh.indices.size() % 4;
    uint32_t verticesSize = static_cast<uint32_t>(mesh.vertices.size());

    //Fix indices
    auto& indices = mesh.indices;
    switch (indices.size() % 4)
    {
    case 0:
        break;
    case 1:
        for (int i = 0; i < 3; i++)
            indices.push_back(verticesSize - 1);
        break;
    case 2:
        indices.pop_back();
        indices.push_back(verticesSize - 2);
        indices.push_back(verticesSize - 1);
        indices.push_back(verticesSize - 1);
        break;
    case 3:
        indices.push_back(verticesSize);
        break;
    default:
        throw std::exception();
    }

    //Fix vertices
    if (meshLastBlockSize == 3)
    {
        auto& vertices = mesh.vertices;
        auto p0 = vertices[verticesSize - 3];
        auto p1 = vertices[verticesSize - 2];
        auto p2 = vertices[verticesSize - 1];
        vertices.pop_back();
        vertices.pop_back();
        vertices.pop_back();
        vertices.push_back(p0);
        vertices.push_back({ (1.0f / 3.0f) * p0 + (2.0f / 3.0f) * p1 });
        vertices.push_back({ (2.0f / 3.0f) * p1 + (1.0f / 3.0f) * p2 });
        vertices.push_back(p2);
    }
}

std::vector<uint32_t> MeshGenerator::BezierCurve::GenerateIndices(unsigned int verticesSize)
{
    std::vector<uint32_t> indices;
    indices.push_back(0);

    for (unsigned int i = 1; i < verticesSize; i++)
    {
        indices.push_back(i);
        if (i % 3 == 0)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

MeshGenerator::GeneratedMesh<Algebra::Vector4> MeshGenerator::BezierCurve::
	GenerateMesh(const std::vector<Algebra::Vector4>& controlPoints)
{
    GeneratedMesh<Algebra::Vector4> result;
    result.renderingMode = RenderingMode::Patches;
    result.shaderType = AvailableShaders::BezierCurveC0;

    if (controlPoints.empty())
    {
        return result;
    }

    result.vertices = controlPoints;
    result.indices = GenerateIndices(static_cast<unsigned int>(controlPoints.size()));

    FixMesh(result);

    return result;
}