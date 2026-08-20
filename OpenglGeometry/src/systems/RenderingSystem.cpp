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

		cameraComponent.cameraHandling->HandleInput(cameraComponent, m_Viewport.GetData());

		sceneContext.CameraPosition = cameraComponent.cameraHandling->GetPosition();
		sceneContext.ProjectionMatrix = cameraComponent.projectionMatrix;
		sceneContext.ViewMatrix = cameraComponent.viewMatrix;
	}

	m_Renderer->SetSceneContext(sceneContext);

	RenderEntities(m_Scene->GetAllEntitiesWith<MeshComponent>(
		Excluded<IsInvisibleTag, IsTransparentTag>()));

	m_Renderer->SetDepthMode(DepthMode::NoDepth);
	RenderEntities(m_Scene->GetAllEntitiesWith<MeshComponent, IsTransparentTag>(
		Excluded<IsInvisibleTag>()));
	m_Renderer->SetDepthMode(DepthMode::Depth);
}
