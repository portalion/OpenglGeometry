#include "NotificationSystem.h"
#include "scene/Entity.h"
#include "scene/Tags.h"
#include "scene/Components.h"


NotificationSystem::NotificationSystem(Ref<Scene> scene)
{
	m_Scene = scene;
}

void NotificationSystem::Process()
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
