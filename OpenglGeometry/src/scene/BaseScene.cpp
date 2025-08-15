#include "BaseScene.h"
#include "Entity.h"
#include "core/Globals.h"

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

	//Triangle entity setup
	{
		auto triangle = CreateEntity();
		std::vector<float> vertices = {
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f,  0.5f, 0.0f
		};
		std::vector<uint32_t> indices = { 0, 1, 2 };
		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "position" }
		};

		auto VAO = VertexArray::CreateWithBuffers(vertices, indices, layout);
		auto& smc = triangle.AddComponent<MeshComponent>();
		smc.mesh = VAO;
	}
	
	//Torus entity setup
	{
		auto torus = CreateEntity();
		torus.AddComponent<IdComponent>();
		torus.AddTag<IsDirtyTag>();
		torus.AddComponent<TorusGenerationComponent>();
		torus.AddComponent<PositionComponent>().position = { 0.f, 0.f, -5.f };
		torus.AddComponent<RotationComponent>();
		torus.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };
	}
}
