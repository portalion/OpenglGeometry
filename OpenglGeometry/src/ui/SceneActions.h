#pragma once
#include <algorithm>
#include <vector>
#include "core/Base.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "interfaces/ICamera.h"
#include "archetypes/CursorArchetypeCreation.h"

namespace GUI
{
	inline auto GetSceneObjects(Ref<Scene> scene)
	{
		return scene->GetAllEntitiesWith<IdComponent, NameComponent, ObjectTypeComponent>();
	}

	inline void SelectAll(Ref<Scene> scene)
	{
		for (Entity entity : GetSceneObjects(scene))
		{
			if (!entity.HasComponent<IsSelectedTag>())
			{
				entity.AddTag<IsSelectedTag>();
			}
		}
	}

	inline void DeselectAll(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag>())
		{
			entity.RemoveTag<IsSelectedTag>();
		}
	}

	inline void DeleteSelected(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag>())
		{
			entity.AddTag<ToBeDestroyedTag>();
		}
	}

	inline bool AnythingSelected(Ref<Scene> scene)
	{
		return !scene->GetAllEntitiesWith<IsSelectedTag>().empty();
	}

	inline Entity SingleSelected(Ref<Scene> scene)
	{
		Entity result;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, NameComponent>())
		{
			if (result.IsValid())
			{
				return Entity{};
			}

			result = entity;
		}

		return result;
	}

	inline bool CanRename(Ref<Scene> scene)
	{
		return SingleSelected(scene).IsValid();
	}

	inline void FocusSelected(Ref<Scene> scene)
	{
		std::vector<Algebra::Vector4> points;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>())
		{
			points.push_back(entity.GetComponent<PositionComponent>().position);
		}

		if (points.empty())
		{
			points.push_back(Archetypes::GetCursorPosition(scene.get()));
		}

		Algebra::Vector4 center = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
		for (const auto& point : points)
		{
			center = center + Algebra::Vector4(point.x, point.y, point.z, 0.f);
		}
		center = center / static_cast<float>(points.size());

		float radius = 0.f;
		for (const auto& point : points)
		{
			radius = std::max(radius, (Algebra::Vector4(point.x, point.y, point.z, 0.f) - center).Length());
		}

		center.w = 1.f;

		for (Entity entity : scene->GetAllEntitiesWith<CameraComponent>())
		{
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			if (!cameraComponent.active || !cameraComponent.cameraHandling)
			{
				continue;
			}

			cameraComponent.cameraHandling->Focus(center, radius);
		}
	}

	inline void SelectOnly(Ref<Scene> scene, Entity entity)
	{
		DeselectAll(scene);
		entity.AddTag<IsSelectedTag>();
	}
}
