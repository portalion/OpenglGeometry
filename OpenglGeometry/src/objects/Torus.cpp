#include "Torus.h"
#include <imgui/imgui.h>

Algebra::Vector4 Torus::GetPoint(float angleTube, float angleRadius)
{
	return Algebra::Matrix4::RotationY(angleRadius) * 
		Algebra::Vector4(radius + tubeRadius * cosf(angleTube), tubeRadius * sinf(angleTube), 0.f, 1.f);
}

RenderableMesh<PositionVertexData> Torus::GenerateMesh()
{
	RenderableMesh<PositionVertexData> result;

	for (unsigned int i = 0; i < radiusSegments; i++)
	{
		for (unsigned int j = 0; j < tubeSegments; j++)
		{
			PositionVertexData vertex;
			vertex.Position = GetPoint(2 * 3.14f * j / tubeSegments, 2 * 3.14f * i / radiusSegments);
			result.vertices.push_back(vertex);
		}
	}

	for (unsigned int i = 0; i < radiusSegments; i++)
	{
		int iNext = (i + 1) % radiusSegments;
		for (unsigned int j = 0; j < tubeSegments; j++)
		{
			int jNext = (j + 1) % tubeSegments;
			result.indices.push_back(tubeSegments * i + j);
			result.indices.push_back(tubeSegments * i + jNext);
			result.indices.push_back(tubeSegments * i + j);
			result.indices.push_back((tubeSegments * iNext) + j);
		}
	}

	return result;
}

bool Torus::DisplayParameters()
{
	uint32_t min_value = 3;
	uint32_t max_value = 100;

	bool somethingChanged = false;

	somethingChanged |= ImGui::SliderFloat(GenerateLabelWithId("Radius").c_str(), &radius, 1.f, 100.f);
	somethingChanged |= ImGui::SliderFloat(GenerateLabelWithId("Tube Radius").c_str(), &tubeRadius, 1.f, 50.f);
	somethingChanged |= ImGui::SliderScalar(GenerateLabelWithId("Segments").c_str(), ImGuiDataType_U32, &radiusSegments, &min_value, &max_value);
	somethingChanged |= ImGui::SliderScalar(GenerateLabelWithId("Circle Segments").c_str(), ImGuiDataType_U32, &tubeSegments, &min_value, &max_value);

	return somethingChanged;
}

std::shared_ptr<Torus> Torus::Deserialize(const json& j)
{
	auto id = j["id"].get<unsigned int>();
	auto name = j["name"].get<std::string>();
	auto position = Algebra::Vector4(
		j["position"]["x"].get<float>(),
		j["position"]["y"].get<float>(),
		j["position"]["z"].get<float>(), 1.f); 	 
	auto rotation = Algebra::Quaternion(
		j["rotation"]["x"].get<float>(),
		j["rotation"]["y"].get<float>(),
		j["rotation"]["z"].get<float>(),
		j["rotation"]["w"].get<float>());
	auto scale = Algebra::Vector4(
		j["scale"]["x"].get<float>(),
		j["scale"]["y"].get<float>(),
		j["scale"]["z"].get<float>(), 1.f);
	auto result = std::make_shared<Torus>();
	result->InitName(id, name);
	result->SetPosition(position);
	result->SetRotation(rotation);
	result->SetScale(scale);
	result->tubeRadius = j["smallRadius"].get<float>();
	result->radius = j["largeRadius"].get<float>();
	result->radiusSegments = j["samples"]["u"].get<unsigned int>();
	result->tubeSegments = j["samples"]["v"].get<unsigned int>();
	return result;
}

json Torus::Serialize() const
{
	json result;
	result["id"] = id;
	result["objectType"] = "torus";
	result["name"] = name;
	result["position"] = json::object({
		{ "x", GetPosition().x },
		{ "y", GetPosition().y },
		{ "z", GetPosition().z } });
	result["rotation"] = json::object({
		{ "x", GetRotation().x },
		{ "y", GetRotation().y },
		{ "z", GetRotation().z },
		{ "w", GetRotation().w } });
	result["scale"] = json::object({
		{ "x", GetScale().x },
		{ "y", GetScale().y },
		{ "z", GetScale().z } });
	result["samples"] = json::object({
		{ "u", radiusSegments },
		{ "v", tubeSegments } });
	result["smallRadius"] = tubeRadius;
	result["largeRadius"] = radius;

	return result;
}
