#include "DefaultLayout.h"
#include <core/Globals.h>
#include <algorithm>
#include <ui/Utils.h>
#include <ui/LogPanel.h>

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

	ImGuiID logId = 0;
	ImGuiID viewportId = 0;
	ImGui::DockBuilderSplitNode(centralId, ImGuiDir_Down, 0.25f, &logId, &viewportId);

	ImGuiID shapeListId = 0;
	ImGuiID restId = 0;
	ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Down, 0.72f, &restId, &shapeListId);

	ImGuiID cursorId = 0;
	ImGuiID inspectorId = 0;
	ImGui::DockBuilderSplitNode(restId, ImGuiDir_Down, 0.35f, &cursorId, &inspectorId);

	ImGui::DockBuilderDockWindow(GUI::ShapeListWindow, shapeListId);
	ImGui::DockBuilderDockWindow(GUI::InspectorWindow, inspectorId);
	ImGui::DockBuilderDockWindow(GUI::CursorPanelWindow, cursorId);
	ImGui::DockBuilderDockWindow(GUI::LogPanelWindow, logId);

	dockspace.FinishCreation();
}