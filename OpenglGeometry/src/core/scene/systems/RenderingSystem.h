#pragma once
#include "core/Base.h"
#include "Algebra.h"

class Scene;
class Shader;
class Entity;

class RenderingSystem
{
private:
	Ref<Scene> scene;
	Ref<Shader> defaultShader;

	Algebra::Matrix4 GetModelMatrix(Entity entity);
public:
	RenderingSystem(Ref<Scene> scene);

	void Process();
};

