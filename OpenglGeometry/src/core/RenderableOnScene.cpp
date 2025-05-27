#include "RenderableOnScene.h"
#include <managers/IdManager.h>

RenderableOnScene::RenderableOnScene()
	: Renderable<PositionVertexData>(VertexDataType::PositionVertexData)
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
	somethingChanged = false;
}

void RenderableOnScene::InitName()
{
	auto typeName = GetTypeName();
	id = IdManager::GetInstance().GetNewId(typeName);
	name = GetTypeName() + ' ' + std::to_string(id);
}

void RenderableOnScene::InitName(unsigned int id, std::string name)
{
	auto typeName = GetTypeName();
	this->id = id;
	this->name = name;
	IdManager::GetInstance().SaveId(typeName, id);
}

std::string RenderableOnScene::GenerateLabelWithId(std::string label)
{
	return label + "##" + GetTypeName() + std::to_string(id);
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
	auto position = this->GetPosition();
	ImGui::InputFloat3(GenerateLabelWithId("Position").c_str(), &position[0]);
	SetPosition(position);
	somethingChanged |= DisplayParameters();
}
