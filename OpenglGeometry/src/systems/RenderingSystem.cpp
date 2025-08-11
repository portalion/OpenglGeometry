#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>

#include "renderer/VertexArray.h"
#include <GL/glew.h>

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

RenderingSystem::RenderingSystem(Ref<Scene> m_Scene)
	: m_Scene(m_Scene)
{
	m_Renderer = CreateRef<Renderer>();
	m_Renderer->SetShader(AvailableShaders::Default);
}

void RenderingSystem::Process()
{
	for (auto entity : m_Scene->GetAllEntitiesWith<CameraComponent>())
	{
		Entity e{ entity, m_Scene.get() };
		auto& cameraComponent = e.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		//TODO: Change it, for now it is working bad
		auto viewMatrix = GetModelMatrix(e);

		m_Renderer->SetCamera(cameraComponent.projectionMatrix, viewMatrix);
	}

	for (auto entity : m_Scene->GetAllEntitiesWith<MeshComponent>())
	{
		Entity e{ entity, m_Scene.get() };

		auto& meshComponent = e.GetComponent<MeshComponent>();
		m_Renderer->SetShader(meshComponent.shaderType);

		auto modelMatrix = GetModelMatrix(e);
		m_Renderer->SetTransformations(modelMatrix);
		m_Renderer->SetMesh(meshComponent.mesh);
		m_Renderer->Render(RenderingMode::Triangles);
	}
}
