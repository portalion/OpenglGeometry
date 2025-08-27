#pragma once
#include "core/Base.h"
#include "Algebra.h"
#include "renderer/Renderer.h"
#include "ISystem.h"

class Scene;
class Shader;
class Entity;

class RenderingSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<Renderer> m_Renderer;

	Algebra::Matrix4 GetModelMatrix(Entity entity);
public:
	RenderingSystem(Ref<Scene> m_Scene);

	void Process();
};

