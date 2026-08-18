#include "DefaultLayout.h"
#include <core/Globals.h>
#include <algorithm>
#include <ui/Utils.h>

void GUI::Layout::Default(Dockspace& dockspace)
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	auto dockspaceId = dockspace.StartCreation(viewport->WorkSize);

	const float ratio = viewport->WorkSize.x > 0.f
		? std::clamp(static_cast<float>(Globals::rightInterfaceWidth) / viewport->WorkSize.x, 0.15f, 0.5f)
		: 0.25f;

	ImGuiID rightId = 0;
	ImGuiID centralId = 0;
	ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, ratio, &rightId, &centralId);

	ImGuiID rightTopId = 0;
	ImGuiID rightBottomId = 0;
	ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Down, 0.5f, &rightBottomId, &rightTopId);

	ImGui::DockBuilderDockWindow(GUI::ShapeListWindow, rightTopId);
	ImGui::DockBuilderDockWindow(GUI::InspectorWindow, rightBottomId);

	dockspace.FinishCreation();
}