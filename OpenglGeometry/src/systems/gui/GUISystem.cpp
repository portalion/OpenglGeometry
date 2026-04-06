#include "GUISystem.h"
#include "UI/GUI.h"

GUISystem::GUISystem(Ref<Scene> scene)
	:m_Scene(scene)
{
	m_Cursor = m_Scene->CreateEntity();
	Archetypes::AddShapeToEntity(m_Cursor, "Cursor");
	m_Cursor.AddComponent<PositionComponent>();
	auto& meshComponent = m_Cursor.AddComponent<MeshComponent>();
	meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Cursor);
	meshComponent.shaderTypes.push_back(AvailableShaders::Default);
	meshComponent.renderingMode = RenderingMode::Lines;
}

void GUISystem::Process()
{
	if (!m_Cursor.HasComponent<PositionComponent>())
	{
		m_Cursor.AddComponent<PositionComponent>();
	}

	GUI::DisplayCreationButtons(m_Scene, m_Cursor.GetComponent<PositionComponent>().position);
	GUI::DisplayShapeList(m_Scene);
	GUI::DisplaySelectedShapesProperties(m_Scene);
}
