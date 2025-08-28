#include "SystemPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

#include "RenderingSystem.h"
#include "MeshGeneratingSystem.h"
#include "NotificationSystem.h"
#include "GUISystem.h"
#include "RemovalSystem.h"

SystemPipeline::SystemPipeline(Ref<Scene> m_Scene)
{
	this->m_Scene = m_Scene;

	m_Systems =
	{
		CreateRef<NotificationSystem>(m_Scene),
		CreateRef<RemovalSystem>(m_Scene),
		CreateRef<GUISystem>(m_Scene),
		CreateRef<MeshGeneratingSystem>(m_Scene),
		CreateRef<RenderingSystem>(m_Scene)
	};
}

SystemPipeline::~SystemPipeline()
{
}

void SystemPipeline::Update()
{
	for(auto& system : m_Systems)
	{
		system->Process();
	}
}
