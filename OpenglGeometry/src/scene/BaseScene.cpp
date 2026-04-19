#include "BaseScene.h"
#include "Entity.h"
#include "core/Globals.h"
#include "archetypes/Archetypes.h"
#include "scene/Components.h"
#include <core/DragCamera.h>

BaseScene::BaseScene()
{
	//Base Camera setup
	{
		float aspect = static_cast<float>(Globals::startingSceneWidth) / static_cast<float>(Globals::startingSceneHeight);
		auto camera = CreateEntity();
		auto& cc = camera.AddComponent<CameraComponent>();
		cc.active = true;
		cc.projectionMatrix = Algebra::Matrix4::Projection(aspect, 0.1f, 10000.0f, 3.14f / 2.f);
		cc.cameraHandling = CreateRef<DragCamera>();
		camera.AddComponent<NameComponent>().name = "camera";
	}

	{
		auto grid = CreateEntity();
		auto& mc = grid.AddComponent<MeshComponent>();
		mc.renderingMode = RenderingMode::Lines;
		mc.shaderTypes = { AvailableShaders::InfiniteGrid };
		mc.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Grid);
	}
}
