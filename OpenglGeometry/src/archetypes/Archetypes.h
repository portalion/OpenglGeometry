#pragma once
#include "scene/Scene.h"
#include "scene/Entity.h"

//TODO: Move it to other files etc

namespace Archetypes
{
	Entity CreateTorus(Scene* scene)
	{
		auto resultTorus = scene->CreateEntity();

		auto id = resultTorus.AddComponent<IdComponent>().id;
		resultTorus.AddComponent<NameComponent>().name = "Torus" + std::to_string(id);

		resultTorus.AddTag<IsDirtyTag>();
		resultTorus.AddComponent<TorusGenerationComponent>();

		resultTorus.AddComponent<PositionComponent>().position = { 0.f, 0.f, 0.f };
		resultTorus.AddComponent<RotationComponent>();
		resultTorus.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };

		return resultTorus;
	}
}
