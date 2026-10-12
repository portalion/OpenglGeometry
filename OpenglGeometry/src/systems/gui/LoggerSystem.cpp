#include "LoggerSystem.h"
#include <imgui/imgui.h>
#include <ui/LogPanel.h>

void LoggerSystem::Process()
{
	// Appends to the "View" menu already submitted by GUISystem this frame
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Log", nullptr, &m_Show);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (m_Show)
	{
		GUI::DrawLogPanel(&m_Show);
	}
}
