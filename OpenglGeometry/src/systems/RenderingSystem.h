#pragma once
#include "core/Base.h"
#include "Algebra.h"
#include "renderer/Renderer.h"
#include "interfaces/ISystem.h"
#include "renderer/UniformApplier.h"
#include "scene/Components.h"
#include "scene/Entity.h"

class Scene;
class Shader;
class Viewport;

class RenderingSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<Renderer> m_Renderer;
	UniformApplier m_UniformApplier;

	Viewport& m_Viewport;

	template<typename Entities>
	void RenderEntities(Entities entities, const Algebra::Vector4& eyeTint = Algebra::Vector4(0.f, 0.f, 0.f, 0.f));

	void RenderMono(const SceneContext& context);
	void RenderStereo(const SceneContext& base, const CameraComponent& camera);
public:
	RenderingSystem(Ref<Scene> m_Scene, Viewport& viewport);

	void Process();
};

template<typename Entities>
inline void RenderingSystem::RenderEntities(Entities entities, const Algebra::Vector4& eyeTint)
{
	const bool stereoEye = eyeTint.x != 0.f || eyeTint.y != 0.f || eyeTint.z != 0.f;

	for (Entity entity : entities)
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();

		EntityContext context;
		m_UniformApplier.PerformFunctions(entity, context);

		if (entity.HasComponent<IsSelectedTag>())
		{
			context.Color = Globals::selectionColor;
		}

		if (stereoEye)
		{
			const float luminance = 0.299f * context.Color.x
				+ 0.587f * context.Color.y + 0.114f * context.Color.z;
			context.Color = Algebra::Vector4(
				luminance * eyeTint.x, luminance * eyeTint.y, luminance * eyeTint.z, context.Color.w);
		}

		m_Renderer->SetMesh(meshComponent.mesh);
		for (auto shaderType : meshComponent.shaderTypes)
		{
			m_Renderer->SetShader(shaderType);
			m_Renderer->Render(meshComponent.renderingMode, context);
		}
	}
}
