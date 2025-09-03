#pragma once
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include <concepts>
#include <iterator>

namespace Archetypes
{
	inline Entity AddShapeToEntity(Entity entity, std::string shapeName)
	{
		auto id = entity.AddComponent<IdComponent>().id;
		entity.AddComponent<NameComponent>().name = shapeName + ' ' + std::to_string(id);

		return entity;
	}

	inline Entity AddVirtualToEntity(Entity entity, Entity parent)
	{
		auto& vec = entity.AddComponent<VirtualEntityComponent>();
		vec.realEntity = parent;
		parent.AddComponent<IsParentOfVirtualEntitiesComponent>().virtualEntity = entity;
		
		return entity;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddNotifiersToEntityContainer(Entity parent, std::list<Entity>& entityContainer,
		Iter pointsBegin, const Iter& pointsEnd)
	{
		for (auto it = pointsBegin; it != pointsEnd; )
		{
			if (!it->IsValid() || !it->HasComponent<NotificationComponent>())
			{
				continue;
			}

			auto& notificationList = it->GetComponent<NotificationComponent>().entitiesToNotify;
			notificationList.push_back(parent);
			entityContainer.push_back(*it);

			it++;
		}

		return parent;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddLineToEntity(Entity entity, Iter pointsBegin, const Iter& pointsEnd)
	{
		auto& controlPoints = entity.AddComponent<LineGenerationComponent>().controlPoints;

		AddNotifiersToEntityContainer(entity, controlPoints, pointsBegin, pointsEnd);

		return entity;
	}
}
