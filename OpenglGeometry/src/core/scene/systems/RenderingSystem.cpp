#include "RenderingSystem.h"
#include "core/scene/Scene.h"
#include "core/scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>

Algebra::Matrix4 RenderingSystem::GetModelMatrix(Entity entity)
{
	Algebra::Matrix4 result = Algebra::Matrix4::Identity();
	if (entity.HasComponent<PositionComponent>())
	{
		auto& position = entity.GetComponent<PositionComponent>().position;
		result = Algebra::Matrix4::Translation(position) * result;
	}

	if (entity.HasComponent<ScaleComponent>())
	{
		auto& scale = entity.GetComponent<ScaleComponent>().scale;
		result = Algebra::Matrix4::DiagonalScaling(scale.x, scale.y, scale.z) * result;
	}

	if (entity.HasComponent<RotationComponent>())
	{
		auto& rotation = entity.GetComponent<RotationComponent>().rotation;
		result = rotation.ToMatrix() * result;
	}

	return result;
}

RenderingSystem::RenderingSystem(Ref<Scene> scene)
	: scene(scene)
{
	defaultShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
}

void RenderingSystem::Process()
{
	if (!defaultShader) return;
	defaultShader->Bind();
	defaultShader->SetUniformVec4f("u_color", Globals::defaultPointsColor);

	for (auto entity : scene->GetAllEntitiesWith<CameraComponent>())
	{
		Entity e{ entity, scene.get() };
		auto cameraComponent = e.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		auto viewMatrix = GetModelMatrix(e);

		defaultShader->SetUniformMat4f("u_projectionMatrix", cameraComponent.projectionMatrix);
		defaultShader->SetUniformMat4f("u_viewMatrix", viewMatrix);
	}
}
