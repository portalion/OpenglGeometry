#include "RenderableOnScene.h"
#include <managers/IdManager.h>

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

	std::string buffer;
	ImGui::InputText(("Shape name##" + GetName()).c_str(), &buffer);
	ImGui::InputFloat3(("Position##" + GetName()).c_str(), &position[0]);
	somethingChanged |= DisplayParameters();
}
