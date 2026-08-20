#include "BaseScene.h"
#include "Entity.h"
#include "core/Globals.h"
#include "archetypes/Archetypes.h"
#include "scene/Components.h"
#include <core/OrbitCamera.h>

BaseScene::BaseScene()
{
	//Base Camera setup
	{
		float aspect = static_cast<float>(Globals::startingSceneWidth) / static_cast<float>(Globals::startingSceneHeight);
		auto camera = CreateEntity();
		auto& cc = camera.AddComponent<CameraComponent>();
		cc.active = true;
		cc.projectionMatrix = Algebra::Matrix4::Projection(aspect, Globals::cameraNearPlane, Globals::cameraFarPlane, Globals::cameraFieldOfView);
		cc.cameraHandling = CreateRef<OrbitCamera>(
			Globals::startingCameraTarget, Globals::startingCameraDistance, Globals::startingCameraPitch);
		camera.AddComponent<NameComponent>().name = "camera";
	}

	{
		auto grid = CreateEntity();
		auto& mc = grid.AddComponent<MeshComponent>();
		mc.renderingMode = RenderingMode::Lines;
		mc.shaderTypes = { AvailableShaders::InfiniteGrid };
		mc.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Grid);
		grid.AddTag<IsTransparentTag>();
	}

	Archetypes::CreateCursor(this);
}
