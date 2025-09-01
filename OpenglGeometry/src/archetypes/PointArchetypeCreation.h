#pragma once
#include "SimpleArchetypeCreation.h"
#include <managers/StaticMeshManager.h>

namespace Archetypes
{
	inline Entity AddPointToEntity(Entity entity)
	{
		entity.AddComponent<PositionComponent>().position.Set({ 0.f, 0.f, 0.f });
		entity.AddComponent<NotificationComponent>();
		auto& meshComponent = entity.AddComponent<MeshComponent>();
		meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Square);
		meshComponent.shaderTypes.push_back(AvailableShaders::Default);

		return entity;
	}

	inline Entity CreatePoint(Scene* scene)
	{
		auto resultPoint = scene->CreateEntity(); 

		AddShapeToEntity(resultPoint, "Point");
		AddPointToEntity(resultPoint);

		return resultPoint;
	}
}
