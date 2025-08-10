#include "Entity.h"

Entity::Entity(entt::entity entity, Scene* m_Scene)
	: m_EntityHandle(entity), m_Scene(m_Scene) 
{
}
