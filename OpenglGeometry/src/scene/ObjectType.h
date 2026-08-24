#pragma once
#include "core/ObjectType.h"
#include "Entity.h"

struct ObjectTypeComponent
{
	ObjectType type = ObjectType::Point;

	ObjectTypeComponent() = default;
	ObjectTypeComponent(ObjectType type) : type(type) { }
	ObjectTypeComponent(const ObjectTypeComponent& other) = default;
};

inline std::optional<ObjectType> GetObjectType(Entity entity)
{
	if (!entity.HasComponent<ObjectTypeComponent>())
	{
		return std::nullopt;
	}

	return entity.GetComponent<ObjectTypeComponent>().type;
}
