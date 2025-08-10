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
		auto cameraComponent = e.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		auto viewMatrix = GetModelMatrix(e);

		m_Renderer->SetCamera(cameraComponent.projectionMatrix, viewMatrix);
	}

	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	uint32_t indices[3] = { 0, 1, 2 };
	auto indexBuffer = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint32_t));

	auto vertexBuffer = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
	BufferLayout layout = {
		{ ShaderDataType::Float3, "position" }
	};
	vertexBuffer->SetLayout(layout);
	auto vertexArray = CreateRef<VertexArray>();
	vertexArray->AddVertexBuffer(vertexBuffer);
	vertexArray->SetIndexBuffer(indexBuffer);

	m_Renderer->SetMesh(vertexArray);
	m_Renderer->Render(RenderingMode::Triangles);
}
