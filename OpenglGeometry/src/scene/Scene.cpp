#include "Scene.h"
#include "Entity.h"

#include "Components.h"

void Scene::OnPositionCreated(entt::registry& registry, entt::entity entity)
{
	auto& position = registry.get<PositionComponent>(entity);
	position.position.entity = Entity{ entity, this };
}

Scene::Scene()
{
	this->m_Registry.on_construct<PositionComponent>()
		.connect<&Scene::OnPositionCreated>(this);
}

Scene::~Scene()
{
	this->m_Registry.on_construct<PositionComponent>()
		.disconnect<&Scene::OnPositionCreated>(this);
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
