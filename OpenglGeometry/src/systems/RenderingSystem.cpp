#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>

#include "renderer/VertexArray.h"
#include <GL/glew.h>
#include "scene/Components.h"

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

		//cameraComponent.cameraHandling->HandleInput();

		//TODO: Change it, for now it is working bad
		RendererContext cameraUniforms;
		m_UniformApplier.PerformFunctions(entity, cameraUniforms);

		auto s = ShaderManager::GetInstance().GetShader(AvailableShaders::InfiniteGrid);
		s->Bind();
		s->SetUniformVec4f("uCameraPos", Algebra::Vector4() * cameraUniforms.Position);
		m_Renderer->SetCamera(cameraComponent.projectionMatrix, cameraUniforms.Position * cameraUniforms.Rotation * cameraUniforms.Scale);
	}

	for (Entity entity : m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>()))
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();

		RendererContext context;
		m_UniformApplier.PerformFunctions(entity, context);
		m_Renderer->SetMesh(meshComponent.mesh);
		for(auto shaderType : meshComponent.shaderTypes)
		{
			m_Renderer->SetShader(shaderType);
			m_Renderer->Render(meshComponent.renderingMode, context);
		}
	}
}
