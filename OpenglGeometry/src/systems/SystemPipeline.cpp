#include "SystemPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

#include "UI/GUI.h"

SystemPipeline::SystemPipeline(Ref<Scene> m_Scene)
{
	this->m_Scene = m_Scene;
	m_RenderingSystem = CreateRef<RenderingSystem>(m_Scene);
	m_MeshGeneratingSystem = CreateRef<MeshGeneratingSystem>(m_Scene);
}

SystemPipeline::~SystemPipeline()
{
}

void SystemPipeline::Update()
{
	GUI::DisplayCreationButtons(m_Scene);
	GUI::DisplayShapeList(m_Scene);

	if (m_MeshGeneratingSystem)
	{
		m_MeshGeneratingSystem->Process();
	}

	if (m_RenderingSystem)
	{
		m_RenderingSystem->Process();
	}
}
