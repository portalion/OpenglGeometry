#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddPolylineToEntity(Entity entity, Iter pointsBegin, const Iter& pointsEnd)
	{
		entity.AddTag<IsDirtyTag>();

		AddLineToEntity(entity, pointsBegin, pointsEnd);

		return entity;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity CreatePolyline(Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		auto resultPolyline = scene->CreateEntity();

		AddShapeToEntity(resultPolyline, "Polyline");
		AddPolylineToEntity(resultPolyline, pointsBegin, pointsEnd);

		return resultPolyline;
	}
}
