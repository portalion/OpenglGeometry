#pragma once
#include "SimpleArchetypeCreation.h"
#include <core/Globals.h>
#include <managers/StaticMeshManager.h>

namespace Archetypes
{
	inline Entity CreateCursor(Scene* scene)
	{
		auto cursor = scene->CreateEntity();

		cursor.AddComponent<NameComponent>().name = "Cursor";
		cursor.AddTag<CursorTag>();
		cursor.AddComponent<PositionComponent>();
		cursor.AddComponent<ColorComponent>().color = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);

		auto& meshComponent = cursor.AddComponent<MeshComponent>();
		meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Cursor);
		meshComponent.shaderTypes.push_back(AvailableShaders::Cursor);
		meshComponent.renderingMode = RenderingMode::Lines;

		return cursor;
	}

	inline Algebra::Vector4 GetCursorPosition(Scene* scene)
	{
		for (Entity cursor : scene->GetAllEntitiesWith<CursorTag, PositionComponent>())
		{
			return cursor.GetComponent<PositionComponent>().position;
		}

		return Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
	}

	inline Entity CreateSelectionCentreMarker(Scene* scene)
	{
		auto marker = scene->CreateEntity();

		marker.AddTag<SelectionCentreTag>();
		marker.AddTag<IsInvisibleTag>();
		marker.AddComponent<PositionComponent>();
		marker.AddComponent<ColorComponent>().color = Globals::selectionColor;

		auto& meshComponent = marker.AddComponent<MeshComponent>();
		meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Crosshair);
		meshComponent.shaderTypes.push_back(AvailableShaders::Point);
		meshComponent.renderingMode = RenderingMode::Lines;

		return marker;
	}
}
