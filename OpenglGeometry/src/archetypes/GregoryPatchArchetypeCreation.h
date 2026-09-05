#pragma once
#include <array>
#include <algorithm>
#include "SimpleArchetypeCreation.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"

namespace Archetypes
{
	inline void AddNotificationFromPoint(Entity point, Entity gregory)
	{
		if (!point.IsValid() || !point.HasComponent<NotificationComponent>())
		{
			return;
		}

		auto& notify = point.GetComponent<NotificationComponent>().entitiesToNotify;
		if (std::find(notify.begin(), notify.end(), gregory) == notify.end())
		{
			notify.push_back(gregory);
		}
	}

	inline void AddNotificationToGregory(Entity gregory, const std::array<GregoryHoleSide, 3>& holeSides = {})
	{
		for (const auto& side : holeSides)
		{
			for (Entity point : side.curve)
			{
				AddNotificationFromPoint(point, gregory);
			}
			for (Entity point : side.inner)
			{
				AddNotificationFromPoint(point, gregory);
			}
		}
	}

	inline Entity CreateGregoryPatch(Scene* scene,
		const std::array<std::array<Algebra::Vector4, 20>, 3>& subPatches,
		const std::array<GregoryHoleSide, 3>& holeSides = {})
	{
		auto entity = scene->CreateEntity();

		AddShapeToEntity(entity, ObjectType::GregoryPatch);

		auto& generation = entity.AddComponent<GregoryPatchGenerationComponent>();
		generation.subPatches = subPatches;
		generation.holeSides = holeSides;

		entity.AddComponent<ColorComponent>().color = Algebra::Vector4(0.2f, 0.55f, 1.f, 1.f);
		entity.AddTag<IsDirtyTag>();

		auto tangentVectors = scene->CreateEntity();
		AddVirtualToEntity(tangentVectors, entity);
		tangentVectors.AddComponent<GregoryTangentComponent>().fillSide = true;
		tangentVectors.AddComponent<ColorComponent>().color = Algebra::Vector4(1.f, 0.1f, 0.9f, 1.f);
		tangentVectors.AddTag<IsInvisibleTag>();

		AddNotificationToGregory(entity, holeSides);

		return entity;
	}
}
