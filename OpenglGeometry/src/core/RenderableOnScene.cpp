#include "RenderableOnScene.h"
#include <managers/IdManager.h>

RenderableOnScene::RenderableOnScene()
	: Renderable<PositionVertexData>()
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

void RenderableOnScene::InitName()
{
	auto typeName = GetTypeName();
	id = IdManager::GetInstance().GetNewId(typeName);
	name = GetTypeName() + ' ' + std::to_string(id);
}

std::string RenderableOnScene::GenerateLabelWithId(std::string label)
{
	return label + "##" + GetTypeName() + std::to_string(id);
}

Algebra::Matrix4 RenderableOnScene::GetModelMatrix() 
{
	return	GetPositionComponent()->GetModelMatrix() *
			GetRotationComponent()->GetModelMatrix() *
			GetScaleComponent()->GetModelMatrix();
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
	//ImGui::InputFloat3(GenerateLabelWithId("Position").c_str(), &position[0]);
	somethingChanged |= DisplayParameters();
}
