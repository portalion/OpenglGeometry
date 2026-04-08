#include "Entity.h"

Entity::Entity(entt::entity entity, Scene* m_Scene)
	: m_EntityHandle(entity), m_Scene(m_Scene) 
{
}

std::vector<entt::id_type> Entity::GetComponentTypes()
{
	std::vector<entt::id_type> result;

	for (auto [id, storage] : m_Scene->m_Registry.storage())
	{
		if (storage.contains(m_EntityHandle))
		{
			result.push_back(id);
		}
	}
	return result;
}