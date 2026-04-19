#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>

#include "renderer/VertexArray.h"
#include <GL/glew.h>
#include "scene/Components.h"
#include <interfaces/ICamera.h>

RenderingSystem::RenderingSystem(Ref<Scene> m_Scene)
	: m_Scene(m_Scene)
{
	m_Renderer = CreateRef<Renderer>();
	m_Renderer->SetShader(AvailableShaders::Default);
}

void RenderingSystem::Process()
{
	SceneContext sceneContext;

	for (Entity entity : m_Scene->GetAllEntitiesWith<CameraComponent>())
	{
		auto& cameraComponent = entity.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		cameraComponent.cameraHandling->HandleInput(cameraComponent);

		EntityContext cameraUniforms;
		m_UniformApplier.PerformFunctions(entity, cameraUniforms);

		sceneContext.CameraPosition = cameraUniforms.Position[3];
		sceneContext.ProjectionMatrix = cameraComponent.projectionMatrix;
		sceneContext.ViewMatrix = cameraComponent.viewMatrix;
	}

	m_Renderer->SetSceneContext(sceneContext);
	
	for (Entity entity : m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>()))
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();

		EntityContext context;
		m_UniformApplier.PerformFunctions(entity, context);
		m_Renderer->SetMesh(meshComponent.mesh);
		for(auto shaderType : meshComponent.shaderTypes)
		{
			m_Renderer->SetShader(shaderType);
			m_Renderer->Render(meshComponent.renderingMode, context);
		}
	}
}
