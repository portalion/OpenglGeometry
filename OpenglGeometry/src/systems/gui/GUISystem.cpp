#include "GUISystem.h"
#include "UI/GUI.h"
#include <archetypes/SimpleArchetypeCreation.h>
#include <managers/StaticMeshManager.h>
#include <ui/layouts/DefaultLayout.h>

GUISystem::GUISystem(Ref<Scene> scene)
	:m_Scene(scene), m_ShapeInspector{ scene } {
}

void GUISystem::Process()
{

	GUI::DrawMenuBar(m_Scene, m_ShowImGuiDemo, [this]() { GUI::Layout::Default(this->m_Dockspace); });
	GUI::DrawToolbar();
	GUI::DrawStatusBar(m_Scene);

	if (!m_Dockspace.Created())
	{
		GUI::Layout::Default(m_Dockspace);
	}

	m_Dockspace.ClaimSize();
	m_Dockspace.FixViewportSize(Globals::viewport);

	if (m_ShowImGuiDemo)
	{
		ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
	}

	GUI::DisplayShapeList(m_Scene);
	m_ShapeInspector.Display();
}
