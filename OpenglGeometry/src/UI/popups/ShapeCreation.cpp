#include "ShapeCreation.h"
#include <imgui/imgui.h>
#include <archetypes/Archetypes.h>

std::vector<Entity> ShapeCreation::GetSelectedPoints()
{
	auto pointsView = m_Scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
	return std::vector<Entity>(pointsView.begin(), pointsView.end());
}

ShapeCreation::ShapeCreation(Ref<Scene> scene)
{
	m_Scene = scene;
	m_Cursor = m_Scene->CreateEntity();
	Archetypes::AddShapeToEntity(m_Cursor, "Cursor");
	m_Cursor.AddComponent<PositionComponent>();
	auto& meshComponent = m_Cursor.AddComponent<MeshComponent>();
	meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Cursor);
	meshComponent.shaderTypes.push_back(AvailableShaders::Default);
	meshComponent.renderingMode = RenderingMode::Lines;
}

void ShapeCreation::Display()
{
	auto cursorPosition = m_Cursor.GetComponent<PositionComponent>().position;
	if (ImGui::MenuItem("Create Torus##Creation menu"))
	{
		Archetypes::CreateTorus(m_Scene.get(), cursorPosition);
	}

	if (ImGui::MenuItem("Create Point##Creation menu"))
	{
		Archetypes::CreatePoint(m_Scene.get(), cursorPosition);
	}

	if (ImGui::MenuItem("Create Polyline##Creation menu"))
	{
		auto selectedPoints = GetSelectedPoints();
		Archetypes::CreatePolyline(m_Scene.get(), selectedPoints.begin(), selectedPoints.end());
	}

	if (ImGui::MenuItem("Create Bezier C0##Creation menu"))
	{
		auto selectedPoints = GetSelectedPoints();
		Archetypes::CreateBezierC0(m_Scene.get(), selectedPoints.begin(), selectedPoints.end());
	}

	if (ImGui::MenuItem("Create Bezier C2##Creation menu"))
	{
		auto selectedPoints = GetSelectedPoints();
		Archetypes::CreateBezierC2(m_Scene.get(), selectedPoints.begin(), selectedPoints.end());
	}

	if (ImGui::MenuItem("Create Interpolated Bezier##Creation menu"))
	{
		auto selectedPoints = GetSelectedPoints();
		Archetypes::CreateInterpolatedBezier(m_Scene.get(), selectedPoints.begin(), selectedPoints.end());
	}
}
