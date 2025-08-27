#include "GUISystem.h"
#include "UI/GUI.h"

GUISystem::GUISystem(Ref<Scene> scene)
	:m_Scene(scene)
{
}

void GUISystem::Process()
{
	GUI::DisplayCreationButtons(m_Scene);
	GUI::DisplayShapeList(m_Scene);
	GUI::DisplaySelectedShapesProperties(m_Scene);
}
