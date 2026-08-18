#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>
#include <core/Viewport.h>
#include <utils/GlCall.h>

#include "renderer/VertexArray.h"
#include <GL/glew.h>
#include "scene/Components.h"
#include <interfaces/ICamera.h>

RenderingSystem::RenderingSystem(Ref<Scene> m_Scene, Viewport& viewport)
	: m_Scene(m_Scene), m_Viewport(viewport)
{
	m_Renderer = CreateRef<Renderer>();
	m_Renderer->SetShader(AvailableShaders::Default);
}

void RenderingSystem::Process()
{
	SceneContext sceneContext;

	// Read the flag before Apply() clears it - the projection depends on the same change.
	const bool viewportChanged = m_Viewport.IsDirty() && m_Viewport.IsValid();
	m_Viewport.Apply();

	for (Entity entity : m_Scene->GetAllEntitiesWith<CameraComponent>())
	{
		auto& cameraComponent = entity.GetComponent<CameraComponent>();
		if (!cameraComponent.active) continue;

		if (viewportChanged)
		{
			cameraComponent.projectionMatrix = Algebra::Matrix4::Projection(
				m_Viewport.Aspect(), Globals::cameraNearPlane, Globals::cameraFarPlane, Globals::cameraFieldOfView);
		}

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
