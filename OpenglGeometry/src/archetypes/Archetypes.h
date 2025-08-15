#pragma once
#include "scene/Scene.h"
#include "scene/Entity.h"

//TODO: Move it to other files etc

namespace Archetypes
{
	inline Entity CreateTorus(Scene* scene)
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

	inline Entity CreatePoint(Scene* scene)
	{
		auto resultPoint = scene->CreateEntity();
		auto id = resultPoint.AddComponent<IdComponent>().id;
		resultPoint.AddComponent<NameComponent>().name = "Point" + std::to_string(id);

		resultPoint.AddComponent<PositionComponent>().position = { 0.f, 0.f, 0.f };

		//TODO: Change to use meshmanager or something
		std::vector<float> vertices = {
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
		};
		std::vector<unsigned int> indices =
		{
			0, 3, 2,
			1, 2, 3
		};
		BufferLayout layout = 
		{
			{ ShaderDataType::Float3, "position" }
		};
		resultPoint.AddComponent<MeshComponent>().mesh = VertexArray::CreateWithBuffers(vertices, indices, layout);

		return resultPoint;
	}
}
