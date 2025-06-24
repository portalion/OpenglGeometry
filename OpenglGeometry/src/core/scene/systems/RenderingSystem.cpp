#include "RenderingSystem.h"
#include "core/scene/Scene.h"
#include "core/scene/Entity.h"

RenderingSystem::RenderingSystem(Ref<Scene> scene)
	: scene(scene)
{
}

void RenderingSystem::Process()
{
	for (auto entity : scene->GetAllEntitiesWith<MeshComponent>())
	{
		Entity e{ entity, scene.get() };
		auto& meshComponent = e.GetComponent<MeshComponent>();
		Renderer r(meshComponent.mesh.vertices, meshComponent.mesh.indices);
		r.Render();
	}
}
