#include "CAMLayout.h"
#include <core/Globals.h>
#include <algorithm>

void GUI::Layout::CAM(Dockspace& dockspace)
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	auto dockspaceId = dockspace.StartCreation(viewport->WorkSize);

	const float ratio = viewport->WorkSize.x > 0.f
		? std::clamp(static_cast<float>(Globals::rightInterfaceWidth) / viewport->WorkSize.x, 0.15f, 0.5f)
		: 0.25f;

	ImGuiID rightId = 0;
	ImGuiID centralId = 0;
	ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, ratio, &rightId, &centralId);

	ImGuiID topId = 0;
	ImGuiID restId = 0;
	ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Down, 0.72f, &restId, &topId);

	ImGuiID bottomId = 0;
	ImGuiID middleId = 0;
	ImGui::DockBuilderSplitNode(restId, ImGuiDir_Down, 0.35f, &bottomId, &middleId);

	ImGui::DockBuilderDockWindow(GUI::CAMTopPanelWindow, topId);
	ImGui::DockBuilderDockWindow(GUI::CAMMiddlePanelWindow, middleId);
	ImGui::DockBuilderDockWindow(GUI::CAMBottomPanelWindow, bottomId);

	dockspace.FinishCreation();
}
