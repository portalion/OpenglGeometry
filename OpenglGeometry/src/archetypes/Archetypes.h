#pragma once
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

//TODO: Move it to other files etc

namespace Archetypes
{
	inline Entity CreateShape(Scene* scene, std::string shapeName)
	{
		auto resultShape = scene->CreateEntity();

		auto id = resultShape.AddComponent<IdComponent>().id;
		resultShape.AddComponent<NameComponent>().name = shapeName + std::to_string(id);

		return resultShape;
	}

	inline Entity CreateTorus(Scene* scene)
	{
		auto resultTorus = CreateShape(scene, "Torus");

		resultTorus.AddTag<IsDirtyTag>();
		resultTorus.AddComponent<TorusGenerationComponent>();

		resultTorus.AddComponent<PositionComponent>().position.Set({ 0.f, 0.f, 0.f });
		resultTorus.AddComponent<RotationComponent>();
		resultTorus.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };

		return resultTorus;
	}

	inline Entity CreatePoint(Scene* scene)
	{
		auto resultPoint = CreateShape(scene, "Point");

		resultPoint.AddComponent<PositionComponent>().position.Set({ 0.f, 0.f, 0.f });

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
