#include "Scene.h"
#include "Entity.h"

Entity Scene::CreateEntity()
{
	Entity entity(m_Registry.create(), this);

	return entity;
}

void Scene::DestroyEntity(Entity entity)
{
	m_Registry.destroy(entity.m_EntityHandle);
}
