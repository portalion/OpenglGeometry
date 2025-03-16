#include "Torus.h"
#include <imgui/imgui.h>

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

void Torus::HandleInput()
{
	uint32_t min_value = 3;
	uint32_t max_value = 100;

	bool somethingChanged = false;

	somethingChanged |= ImGui::SliderFloat("Radius", &radius, 1.f, 100.f);
	somethingChanged |= ImGui::SliderFloat("Tube Radius", &tubeRadius, 1.f, 50.f);
	somethingChanged |= ImGui::SliderScalar("Segments", ImGuiDataType_U32,  &radiusSegments, &min_value, &max_value);
	somethingChanged |= ImGui::SliderScalar("Circle Segments", ImGuiDataType_U32,  &tubeSegments, &min_value, &max_value);

	if (somethingChanged)
	{
		GenerateAndSaveMesh();
	}
}

void Torus::Render()
{
	renderer.Render(GL_LINES);
}
