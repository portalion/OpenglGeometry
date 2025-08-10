#include "SystemPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

SystemPipeline::SystemPipeline(Ref<Scene> m_Scene)
{
	this->m_Scene = m_Scene;
	renderingSystem = CreateRef<RenderingSystem>(m_Scene);
}

SystemPipeline::~SystemPipeline()
{
}

void SystemPipeline::Update()
{
	if (renderingSystem)
	{
		renderingSystem->Process();
	}
}
