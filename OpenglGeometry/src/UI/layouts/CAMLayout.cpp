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

	ImGui::DockBuilderDockWindow(GUI::CAMPathPanelWindow, rightId);

	dockspace.FinishCreation();
}
