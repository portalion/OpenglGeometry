#include "RenderingSystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include <managers/ShaderManager.h>
#include <core/Globals.h>
#include <core/Stereo.h>
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
	const bool viewportChanged = m_Viewport.IsDirty() && m_Viewport.IsValid();
	m_Viewport.Apply();

	SceneContext baseContext;
	const CameraComponent* activeCamera = nullptr;

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

		baseContext.CameraPosition = cameraComponent.cameraHandling->GetPosition();
		baseContext.CameraTarget = cameraComponent.cameraHandling->GetTarget();
		baseContext.ProjectionMatrix = cameraComponent.projectionMatrix;
		baseContext.ViewMatrix = cameraComponent.viewMatrix;
		activeCamera = &cameraComponent;
	}

	if (activeCamera && activeCamera->stereoscopic)
	{
		RenderStereo(baseContext, *activeCamera);
	}
	else
	{
		RenderMono(baseContext);
	}
}

void RenderingSystem::RenderMono(const SceneContext& context)
{
	m_Renderer->SetSceneContext(context);

	RenderEntities(m_Scene->GetAllEntitiesWith<MeshComponent>(
		Excluded<IsInvisibleTag, IsTransparentTag>()));

	m_Renderer->SetDepthMode(DepthMode::NoDepth);
	RenderEntities(m_Scene->GetAllEntitiesWith<MeshComponent, IsTransparentTag>(
		Excluded<IsInvisibleTag>()));
	m_Renderer->SetDepthMode(DepthMode::Depth);
}

void RenderingSystem::RenderStereo(const SceneContext& base, const CameraComponent& camera)
{
	const Stereo::EyePair eyes = Stereo::Compute(base.ViewMatrix, m_Viewport.Aspect(),
		Globals::cameraNearPlane, Globals::cameraFarPlane, Globals::cameraFieldOfView,
		camera.eyeDistance, camera.convergence);

	GLCall(glBlendFunc(GL_ONE, GL_ONE));

	auto drawEye = [&](const Stereo::EyeView& eye, const Algebra::Vector4& tint)
	{
		SceneContext context = base;
		context.ViewMatrix = eye.view;
		context.ProjectionMatrix = eye.projection;

		m_Renderer->SetSceneContext(context);
		RenderEntities(m_Scene->GetAllEntitiesWith<MeshComponent>(
			Excluded<IsInvisibleTag, IsTransparentTag>()), tint);
	};

	drawEye(eyes.left, camera.leftEyeColor);
	GLCall(glClear(GL_DEPTH_BUFFER_BIT));
	drawEye(eyes.right, camera.rightEyeColor);

	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}
