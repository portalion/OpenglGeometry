#include "BaseScene.h"
#include "Entity.h"
#include "core/Globals.h"

BaseScene::BaseScene()
{
	{
		float aspect = static_cast<float>(Globals::startingSceneWidth) / static_cast<float>(Globals::startingSceneHeight);
		auto camera = CreateEntity();
		auto& cc = camera.AddComponent<CameraComponent>();
		cc.active = true;
		cc.projectionMatrix = Algebra::Matrix4::Projection(aspect, 0.1f, 10000.0f, 3.14f / 2.f);
		camera.AddComponent<PositionComponent>().position = Globals::startingCameraPosition;
		camera.AddComponent<RotationComponent>();
		camera.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };
	}

	{
		auto cube = CreateEntity();
		auto& mc = cube.AddComponent<MeshComponent>();
			mc.mesh = {
			.vertices{
				{{  1.f,  1.f, -1.f, 1.f }},
				{{  1.f, -1.f, -1.f, 1.f }},
				{{ -1.f,  1.f, -1.f, 1.f }},
				{{ -1.f, -1.f, -1.f, 1.f }},
				{{  1.f,  1.f,  1.f, 1.f }},
				{{  1.f, -1.f,  1.f, 1.f }},
				{{ -1.f,  1.f,  1.f, 1.f }},
				{{ -1.f, -1.f,  1.f, 1.f }},
			},
			.indices{
				0, 1, 2,   1, 3, 2,
				4, 6, 5,   5, 6, 7,
				2, 3, 6,   3, 7, 6,
				0, 4, 1,   1, 4, 5,
				0, 2, 4,   2, 6, 4,
				1, 5, 3,   3, 5, 7
			}};
	}
}
