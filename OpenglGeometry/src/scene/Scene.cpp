#include "Scene.h"
#include "Entity.h"

#include <vector>

#include "Components.h"

void Scene::OnPositionCreated(entt::registry& registry, entt::entity entity)
{
	auto& position = registry.get<PositionComponent>(entity);
	position.position.entity = Entity{ entity, this };
}

void Scene::OnIdCreated(entt::registry& registry, entt::entity entity)
{
	const auto& idComponent = registry.get<IdComponent>(entity);
	m_EntitiesById[idComponent.id] = entity;
}

void Scene::OnIdDestroyed(entt::registry& registry, entt::entity entity)
{
	const auto& idComponent = registry.get<IdComponent>(entity);

	const auto it = m_EntitiesById.find(idComponent.id);
	if (it != m_EntitiesById.end() && it->second == entity)
	{
		m_EntitiesById.erase(it);
	}
}

Scene::Scene()
{
	this->m_Registry.on_construct<PositionComponent>()
		.connect<&Scene::OnPositionCreated>(this);

	this->m_Registry.on_construct<IdComponent>()
		.connect<&Scene::OnIdCreated>(this);
	this->m_Registry.on_destroy<IdComponent>()
		.connect<&Scene::OnIdDestroyed>(this);
}

Scene::~Scene()
{
	this->m_Registry.on_construct<PositionComponent>()
		.disconnect<&Scene::OnPositionCreated>(this);

	this->m_Registry.on_construct<IdComponent>()
		.disconnect<&Scene::OnIdCreated>(this);
	this->m_Registry.on_destroy<IdComponent>()
		.disconnect<&Scene::OnIdDestroyed>(this);
}

Entity Scene::CreateEntity()
{
	Entity entity(m_Registry.create(), this);

	return entity;
}

void Scene::DestroyEntity(Entity entity)
{
	m_Registry.destroy(entity.m_EntityHandle);
}

void Scene::Clear()
{
	std::vector<Entity> doomed;

	for (Entity entity : GetAllEntitiesWith<IdComponent>())
	{
		doomed.push_back(entity);

		if (entity.HasComponent<IsParentOfVirtualEntitiesComponent>())
		{
			for (Entity virtualEntity : entity.GetComponent<IsParentOfVirtualEntitiesComponent>().virtualEntities)
			{
				doomed.push_back(virtualEntity);
			}
		}
	}

	for (Entity entity : doomed)
	{
		if (entity.IsValid())
		{
			DestroyEntity(entity);
		}
	}
}

bool Scene::HasEntityWithId(ID id) const
{
	return m_EntitiesById.contains(id);
}

Entity Scene::FindEntityById(ID id)
{
	const auto it = m_EntitiesById.find(id);
	if (it == m_EntitiesById.end())
	{
		return Entity{};
	}

	return Entity{ it->second, this };
}
