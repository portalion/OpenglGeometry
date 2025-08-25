#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
	inline Entity AddTorusToEntity(Entity entity)
	{
		entity.AddTag<IsDirtyTag>();
		entity.AddComponent<TorusGenerationComponent>();

		entity.AddComponent<PositionComponent>().position.Set({ 0.f, 0.f, 0.f });
		entity.AddComponent<RotationComponent>();
		entity.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };

		return entity;
	}

	inline Entity CreateTorus(Scene* scene)
	{
		auto resultTorus = scene->CreateEntity();

		AddShapeToEntity(resultTorus, "Torus");
		AddTorusToEntity(resultTorus);

		return resultTorus;
	}
}
