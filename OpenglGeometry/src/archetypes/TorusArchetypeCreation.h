#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
	inline Entity AddTorusToEntity(Entity entity, Algebra::Vector4 position)
	{
		entity.AddTag<IsDirtyTag>();
		entity.AddComponent<TorusGenerationComponent>();

		entity.AddComponent<PositionComponent>().position.Set(position);
		entity.AddComponent<RotationComponent>();
		entity.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };

		return entity;
	}

	inline Entity CreateTorus(Scene* scene, Algebra::Vector4 position)
	{
		auto resultTorus = scene->CreateEntity();

		AddShapeToEntity(resultTorus, "Torus");
		AddTorusToEntity(resultTorus, position);

		return resultTorus;
	}
}
