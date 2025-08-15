#include "BaseScene.h"
#include "Entity.h"
#include "core/Globals.h"
#include "archetypes/Archetypes.h"

BaseScene::BaseScene()
{
	//Base Camera setup
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

	//Torus entity setup
	Archetypes::CreateTorus(this);
}
