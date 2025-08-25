#pragma once
#include "SimpleArchetypeCreation.h"
#include <managers/StaticMeshManager.h>

namespace Archetypes
{
	inline Entity AddPointToEntity(Entity entity)
	{
		entity.AddComponent<PositionComponent>().position.Set({ 0.f, 0.f, 0.f });
		entity.AddComponent<NotificationComponent>();
		entity.AddComponent<MeshComponent>().mesh 
			= StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Square);
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
