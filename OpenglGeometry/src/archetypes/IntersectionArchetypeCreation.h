#pragma once
#include "SimpleArchetypeCreation.h"
#include "core/Globals.h"

namespace Archetypes
{
	inline Entity AddIntersectionCurveToEntity(Entity entity, const IntersectionCurveComponent& data)
	{
		entity.AddComponent<IntersectionCurveComponent>(data);
		entity.AddComponent<ColorComponent>().color = Algebra::Vector4(0.15f, 1.f, 0.55f, 1.f);
		entity.AddTag<IsDirtyTag>();
		return entity;
	}

	inline Entity CreateIntersectionCurve(Scene* scene, const IntersectionCurveComponent& data)
	{
		auto entity = scene->CreateEntity();
		AddShapeToEntity(entity, ObjectType::IntersectionCurve);
		AddIntersectionCurveToEntity(entity, data);
		return entity;
	}
}
