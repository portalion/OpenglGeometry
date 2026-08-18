#include "DockSpace.h"
#include <algorithm>

Dockspace::Dockspace(const char* id)
{
	m_Id = ImHashStr(id);
}

bool Dockspace::Created()
{
	return ImGui::DockBuilderGetNode(m_Id) != nullptr;
}

ImGuiID Dockspace::StartCreation(ImVec2 size)
{
	ImGui::DockBuilderRemoveNode(m_Id);
	ImGui::DockBuilderAddNode(m_Id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockBuilderSetNodeSize(m_Id, size);

	return m_Id;
}

void Dockspace::FinishCreation()
{
	ImGui::DockBuilderFinish(m_Id);
}

void Dockspace::ClaimSize()
{
	ImGui::DockSpaceOverViewport(m_Id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void Dockspace::FixViewportSize(Viewport& viewport)
{
	const ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(m_Id);
	if (centralNode == nullptr)
	{
		return;
	}

	const ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();
	const ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
	const ImRect rect = centralNode->Rect();

	const float framebufferHeight = imGuiViewport->Size.y * scale.y;
	const float left = (rect.Min.x - imGuiViewport->Pos.x) * scale.x;
	const float top = (rect.Min.y - imGuiViewport->Pos.y) * scale.y;
	const float width = rect.GetWidth() * scale.x;
	const float height = rect.GetHeight() * scale.y;

	ViewportData result;
	result.x = static_cast<int>(left);
	result.y = static_cast<int>(framebufferHeight - (top + height));
	result.width = std::max(static_cast<int>(width), 0);
	result.height = std::max(static_cast<int>(height), 0);

	viewport.Change(result);
}
