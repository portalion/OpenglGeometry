#include "RemovalSystem.h"
#include "scene/Tags.h"
#include "scene/Components.h"

RemovalSystem::RemovalSystem(Ref<Scene> scene)
	: m_Scene{scene}
{
}

void RemovalSystem::Process()
{
	for (auto entity : m_Scene->GetAllEntitiesWith<ToBeDestroyedTag>())
	{
		if (entity.HasComponent<IsParentOfVirtualEntitiesComponent>())
		{
			entity.GetComponent<IsParentOfVirtualEntitiesComponent>()
				.virtualEntity.AddTag<ToBeDestroyedTag>();
		}
		m_Scene->DestroyEntity(entity);
	}
}
