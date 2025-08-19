#include "SystemPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

#include "UI/GUI.h"

void SystemPipeline::ProcessSignals()
{
	for (Entity entity : m_Scene->GetAllEntitiesWith<ObserverChangedState, NotificationComponent>())
	{
		entity.RemoveComponent<ObserverChangedState>();
		auto& notificationList = entity.GetComponent<NotificationComponent>().entitiesToNotify;
		for (auto it = notificationList.begin(); it != notificationList.end(); )
		{
			if (!it->IsValid())
			{
				it = notificationList.erase(it);
				continue;
			}

			it->AddTag<IsDirtyTag>();
			it++;
		}
	}	
}

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
	ProcessSignals();

	GUI::DisplayCreationButtons(m_Scene);
	GUI::DisplayShapeList(m_Scene);
	GUI::DisplaySelectedShapesProperties(m_Scene);

	if (m_MeshGeneratingSystem)
	{
		m_MeshGeneratingSystem->Process();
	}

	if (m_RenderingSystem)
	{
		m_RenderingSystem->Process();
	}
}
