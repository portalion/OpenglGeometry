#include "GUISystem.h"
#include "UI/GUI.h"
#include <archetypes/SimpleArchetypeCreation.h>
#include <managers/StaticMeshManager.h>
#include <ui/layouts/DefaultLayout.h>

GUISystem::GUISystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport), m_ShapeInspector{ scene } {
}

void GUISystem::Process()
{
	GUI::DrawMenuBar(m_Scene, m_ShowImGuiDemo, [this]() { GUI::Layout::Default(this->m_Dockspace); });
	GUI::DrawToolbar();
	GUI::DrawStatusBar(m_Scene, m_Viewport.GetData());

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

	GUI::DisplayShapeList(m_Scene);
	m_ShapeInspector.Display();

	GUI::HandleShortcuts(m_Scene);
}
