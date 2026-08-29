#include "ShapeCreation.h"
#include <imgui/imgui.h>
#include <archetypes/Archetypes.h>
#include <ui/SceneActions.h>
#include <ui/Utils.h>
#include <ui/popups/BezierSurfaceDialog.h>

std::vector<Entity> ShapeCreation::GetSelectedPoints()
{
	return GUI::GetSelectedControlPoints(m_Scene);
}

ShapeCreation::ShapeCreation(Ref<Scene> scene)
{
	m_Scene = scene;
}

bool ShapeCreation::ShouldOpen()
{
	return ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_A);
}

void ShapeCreation::Display()
{
	if (ImGui::MenuItem("Create Torus##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::Torus);
	}

	if (ImGui::MenuItem("Create Point##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::Point);
	}

	if (ImGui::MenuItem("Create Polyline##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::Chain);
	}

	if (ImGui::MenuItem("Create Bezier C0##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::BezierC0);
	}

	if (ImGui::MenuItem("Create Bezier C2##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::BezierC2);
	}

	if (ImGui::MenuItem("Create Interpolated Bezier##Creation menu"))
	{
		GUI::CreateShape(m_Scene, ObjectType::InterpolatedC2);
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Create Bezier Surface...##Creation menu"))
	{
		GUI::RequestDialog(GUI::BezierSurfaceDialogTitle);
	}
}
