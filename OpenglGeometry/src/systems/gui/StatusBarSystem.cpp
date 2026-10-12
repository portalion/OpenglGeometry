#include "StatusBarSystem.h"
#include <scene/Scene.h>
#include <core/Viewport.h>
#include <ui/StatusBar.h>

StatusBarSystem::StatusBarSystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport)
{
}

void StatusBarSystem::Process()
{
	GUI::DrawStatusBar(m_Scene, m_Viewport.GetData());
}
