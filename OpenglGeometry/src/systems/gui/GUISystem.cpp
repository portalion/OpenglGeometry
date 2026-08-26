#include "GUISystem.h"
#include "UI/GUI.h"
#include <archetypes/SimpleArchetypeCreation.h>
#include <managers/StaticMeshManager.h>
#include <ui/layouts/DefaultLayout.h>

GUISystem::GUISystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport), m_ShapeList{ scene }, m_ShapeInspector{ scene }
{
	m_Callbacks.resetLayout = [this]() { GUI::Layout::Default(this->m_Dockspace); };
	m_Callbacks.renameSelected = [this]() { this->m_ShapeList.RequestRename(); };
}

void GUISystem::Process()
{
	GUI::DrawMenuBar(m_Scene, m_UiState, m_ShowImGuiDemo, m_ShowParameterSpace, m_Callbacks);
	GUI::DrawToolbar(m_UiState);
	GUI::DrawStatusBar(m_Scene, m_Viewport.GetData(), m_UiState);

	GUI::DrawAllDialogs(m_UiState);

	if (!m_Dockspace.Created())
	{
		GUI::Layout::Default(m_Dockspace);
	}

	m_Dockspace.ClaimSize();

	ViewportData sceneRect;
	if (m_Dockspace.TryGetCentralNodeRect(sceneRect))
	{
		m_Viewport.Change(sceneRect);
	}

	if (m_ShowImGuiDemo)
	{
		ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
	}

	m_ShapeList.Display();
	m_ShapeInspector.Display();
	GUI::DrawCursorPanel(m_Scene, m_UiState, m_Dockspace);

	if (m_ShowParameterSpace)
	{
		GUI::DrawParameterSpace(m_UiState);
	}

	GUI::HandleViewportPicking(m_Scene, m_Dockspace, m_Picking);
	GUI::HandleCursorPlacement(m_Scene, m_Dockspace, m_UiState.cursor, m_CursorPlacement);

	GUI::HandleShortcuts(m_Scene, m_Callbacks);
	GUI::HandleFileShortcuts();
}
