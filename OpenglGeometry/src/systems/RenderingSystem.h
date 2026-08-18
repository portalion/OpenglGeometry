#pragma once
#include "core/Base.h"
#include "Algebra.h"
#include "renderer/Renderer.h"
#include "interfaces/ISystem.h"
#include "renderer/UniformApplier.h"

class Scene;
class Shader;
class Entity;
class Viewport;

class RenderingSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<Renderer> m_Renderer;
	UniformApplier m_UniformApplier;

	Viewport& m_Viewport;
public:
	RenderingSystem(Ref<Scene> m_Scene, Viewport& viewport);

	void Process();
};
