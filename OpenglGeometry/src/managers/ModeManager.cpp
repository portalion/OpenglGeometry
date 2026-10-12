#include "ModeManager.h"
#include <imgui/imgui.h>

ModeManager::ModeManager(Ref<Scene> scene, Viewport& viewport)
	: m_Viewport{ viewport }, m_CurrentScene{ scene }
{
	m_Systems.push_back(CreateUnique<ModellingSystemPipeline>(scene, viewport));
	m_Systems.push_back(CreateUnique<CAMSystemPipeline>(scene, viewport));
}

void ModeManager::ChangeMode()
{
	m_CurrentModeIndex++; 
	if (m_CurrentModeIndex >= m_Systems.size())
		m_CurrentModeIndex = 0;
}

bool ModeManager::ShouldChange()
{
	return ImGui::IsKeyPressed(ImGuiKey_Tab);
}

void ModeManager::Update()
{
	m_Systems[m_CurrentModeIndex]->Update();
}