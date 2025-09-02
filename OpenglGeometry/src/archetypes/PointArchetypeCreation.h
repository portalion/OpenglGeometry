#pragma once
#include "SimpleArchetypeCreation.h"
#include <managers/StaticMeshManager.h>

namespace Archetypes
{
	inline Entity AddPointToEntity(Entity entity, Algebra::Vector4 startingPosition)
	{
		entity.AddComponent<PositionComponent>().position.Set(startingPosition);
		entity.AddComponent<NotificationComponent>();
		auto& meshComponent = entity.AddComponent<MeshComponent>();
		meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Square);
		meshComponent.shaderTypes.push_back(AvailableShaders::Default);

		return entity;
	}

	inline Entity CreatePoint(Scene* scene, Algebra::Vector4 startingPosition)
	{
		auto resultPoint = scene->CreateEntity(); 

		AddShapeToEntity(resultPoint, "Point");
		AddPointToEntity(resultPoint, startingPosition);

		return resultPoint;
	}
}
