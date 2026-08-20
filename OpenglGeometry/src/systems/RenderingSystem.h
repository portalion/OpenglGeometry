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
	void RenderEntities(Entities entities);
public:
	RenderingSystem(Ref<Scene> m_Scene, Viewport& viewport);

	void Process();
};

template<typename Entities>
inline void RenderingSystem::RenderEntities(Entities entities)
{
	for (Entity entity : entities)
	{
		auto& meshComponent = entity.GetComponent<MeshComponent>();

		EntityContext context;
		m_UniformApplier.PerformFunctions(entity, context);
		m_Renderer->SetMesh(meshComponent.mesh);
		for (auto shaderType : meshComponent.shaderTypes)
		{
			m_Renderer->SetShader(shaderType);
			m_Renderer->Render(meshComponent.renderingMode, context);
		}
	}
}
