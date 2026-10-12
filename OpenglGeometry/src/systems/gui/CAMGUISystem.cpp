#include "CAMGUISystem.h"
#include <imgui/imgui.h>
#include <utils/Initialization.h>
#include <core/Log.h>
#include <ui/layouts/CAMLayout.h>
#include <ui/popups/FileDialog.h>

namespace
{
	inline constexpr const char* LoadPathDialogTitle = "Load path##LoadPathDialog";
}

CAMGUISystem::CAMGUISystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport)
{
}

void CAMGUISystem::DrawMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Load path..."))
		{
			m_OpenLoadPathDialog = true;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Exit"))
		{
			glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Reset layout"))
		{
			GUI::Layout::CAM(m_Dockspace);
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void CAMGUISystem::LoadPath(const std::string& file)
{
	Logger::Info("Load path: {}", file);
}

void CAMGUISystem::Process()
{
	DrawMenuBar();

	if (m_OpenLoadPathDialog)
	{
		ImGui::OpenPopup(LoadPathDialogTitle);
		m_OpenLoadPathDialog = false;
	}

	if (GUI::DrawFileDialog(LoadPathDialogTitle, m_PathFile, { "Load" }))
	{
		LoadPath(m_PathFile);
	}

	if (!m_Dockspace.Created())
	{
		GUI::Layout::CAM(m_Dockspace);
	}

	m_Dockspace.ClaimSize();

	ViewportData sceneRect;
	if (m_Dockspace.TryGetCentralNodeRect(sceneRect))
	{
		m_Viewport.Change(sceneRect);
	}

	for (const char* panel : { GUI::CAMTopPanelWindow, GUI::CAMMiddlePanelWindow, GUI::CAMBottomPanelWindow })
	{
		ImGui::Begin(panel);
		ImGui::End();
	}
}
