#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>

#include "renderer/VertexArray.h"
#include <GL/glew.h>
#include "scene/Components.h"

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
	for (Entity entity : m_Scene->GetAllEntitiesWith<CameraComponent>())
	{
		auto& cameraComponent = entity.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		//TODO: Change it, for now it is working bad
		auto viewMatrix = GetModelMatrix(entity);

		m_Renderer->SetCamera(cameraComponent.projectionMatrix, viewMatrix);
	}

	for (Entity entity : m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>()))
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();

		auto modelMatrix = GetModelMatrix(entity);
		m_Renderer->SetTransformations(modelMatrix);
		m_Renderer->SetMesh(meshComponent.mesh);
		for(auto shaderType : meshComponent.shaderTypes)
		{
			m_Renderer->SetShader(shaderType);
			m_Renderer->Render(meshComponent.renderingMode);
		}
	}
}
