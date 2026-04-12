#pragma once
#include <renderer/RendererContext.h>
#include <scene/ComponentFunctionRegistry.h>
#include <renderer/Shader.h>

class UniformApplier : public ComponentFunctionRegistry<EntityContext&>
{
private:
	void PositionApplier(Entity entity, EntityContext& context);
	void RotationApplier(Entity entity, EntityContext& context);
	void ScaleApplier(Entity entity, EntityContext& context);
public:
	UniformApplier();
};