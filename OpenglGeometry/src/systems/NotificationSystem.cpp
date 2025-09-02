#include "NotificationSystem.h"
#include "scene/Entity.h"
#include "scene/Tags.h"
#include "scene/Components.h"


void NotificationSystem::NotifyWithNotificationComponent(Entity entity)
{
	auto& notificationList = entity.GetComponent<NotificationComponent>().entitiesToNotify;
	for (auto it = notificationList.begin(); it != notificationList.end(); )
	{
		if (!it->IsValid())
		{
			it = notificationList.erase(it);
			continue;
		}

		it->AddTag<IsDirtyTag>();
		if(it->HasComponent<VirtualEntityComponent>())
		{
			it->AddTag<ObserverChangedState>();
		}
		it++;
	}
}

void NotificationSystem::NotifyWithVirtualComponent(Entity entity)
{
	auto& parent = entity.GetComponent<VirtualEntityComponent>().realEntity;
	if (parent.IsValid())
	{
		parent.AddTag<IsDirtyTag>();
	}
	else
	{
		entity.AddTag<ToBeDestroyedTag>();
	}
}

NotificationSystem::NotificationSystem(Ref<Scene> scene)
{
	m_Scene = scene;
}

void NotificationSystem::Process()
{

	for (Entity entity : m_Scene->GetAllEntitiesWith<ObserverChangedState, NotificationComponent>())
	{
		NotifyWithNotificationComponent(entity);
	}

	for (Entity entity : m_Scene->GetAllEntitiesWith<ToBeDestroyedTag, NotificationComponent>())
	{
		NotifyWithNotificationComponent(entity);
	}

	for (Entity entity : m_Scene->GetAllEntitiesWith<ObserverChangedState, VirtualEntityComponent>())
	{
		NotifyWithVirtualComponent(entity);
	}

	for(auto entity : m_Scene->GetAllEntitiesWith<ObserverChangedState>())
	{
		entity.RemoveTag<ObserverChangedState>();
	}
}
