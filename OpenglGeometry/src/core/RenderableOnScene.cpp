#include "RenderableOnScene.h"
#include <managers/IdManager.h>

void RenderableOnScene::Scale(float scale)
{
	this->scale *= scale;
}

void RenderableOnScene::Move(Algebra::Vector4 translation)
{
	position += translation;
}

void RenderableOnScene::Rotate(Algebra::Quaternion rotation)
{
	this->rotation = this->rotation * rotation;
	this->rotation.Normalize();
}

void RenderableOnScene::InitName()
{
	auto typeName = GetTypeName();
	id = IdManager::GetInstance().GetNewId(typeName);
	name = GetTypeName() + ' ' + std::to_string(id);
}

void RenderableOnScene::SaveMesh()
{
	const auto& mesh = GenerateMesh();
	renderer.AssignVertices(mesh.vertices);
	renderer.AssignIndices(mesh.indices);
}

std::string RenderableOnScene::GenerateLabelWithId(std::string label)
{
	return label + "##" + GetTypeName() + std::to_string(id);
}

RenderableOnScene::RenderableOnScene()
	:renderer{ VertexDataType::PositionVertexData },
	id{ 0 }
{
	renderingMode = RenderingMode::LINES;
	somethingChanged = true;
}

void RenderableOnScene::Update()
{
	if (somethingChanged)
	{
		SaveMesh();
	}
}

void RenderableOnScene::Render() const
{
	renderer.Render(renderingMode);
}

void RenderableOnScene::DisplayMenu()
{
	ImGui::SeparatorText(GetName().c_str());

	char nameBuffer[128];
	strncpy_s(nameBuffer, name.c_str(), sizeof(nameBuffer));
	nameBuffer[sizeof(nameBuffer) - 1] = '\0';
	if (ImGui::InputText(GenerateLabelWithId("Name").c_str(), nameBuffer, sizeof(nameBuffer)))
	{
		name = std::string(nameBuffer);
	}
	ImGui::InputFloat3(GenerateLabelWithId("Position").c_str(), &position[0]);
	somethingChanged |= DisplayParameters();
}
