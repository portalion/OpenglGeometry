#pragma once
#include "core/Base.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"

namespace GUI
{
	inline void SelectAll(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<NameComponent>())
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
}
