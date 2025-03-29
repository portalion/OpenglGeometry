#include "RenderableOnScene.h"
#include <managers/IdManager.h>

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
	:renderer{ VertexDataType::PositionVertexData }
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
	somethingChanged |= DisplayParameters();
}
