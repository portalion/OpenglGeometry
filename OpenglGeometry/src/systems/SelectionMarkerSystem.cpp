#include "SelectionMarkerSystem.h"

#include "Algebra.h"
#include "archetypes/CursorArchetypeCreation.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "scene/Tags.h"

SelectionMarkerSystem::SelectionMarkerSystem(Ref<Scene> scene)
	: m_Scene{ scene }
{
	Archetypes::CreateSelectionCentreMarker(m_Scene.get());
}

void SelectionMarkerSystem::Process()
{
	Entity marker;
	for (Entity entity : m_Scene->GetAllEntitiesWith<SelectionCentreTag, PositionComponent>())
	{
		marker = entity;
	}

	if (!marker.IsValid())
	{
		return;
	}

	if (marker.HasComponent<SelectionCentreHiddenTag>())
	{
		if (!marker.HasComponent<IsInvisibleTag>())
		{
			marker.AddTag<IsInvisibleTag>();
		}
		return;
	}

	Algebra::Vector4 sum(0.f, 0.f, 0.f, 0.f);
	int count = 0;

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>())
	{
		const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;
		sum = sum + Algebra::Vector4(position.x, position.y, position.z, 0.f);
		count++;
	}

	if (count == 0)
	{
		marker.AddTag<IsInvisibleTag>();
		return;
	}

	const Algebra::Vector4 centre = sum / static_cast<float>(count);
	marker.GetComponent<PositionComponent>().position = Algebra::Vector4(centre.x, centre.y, centre.z, 1.f);

	if (marker.HasComponent<IsInvisibleTag>())
	{
		marker.RemoveTag<IsInvisibleTag>();
	}
}
