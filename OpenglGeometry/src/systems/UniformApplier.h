#pragma once
#include <renderer/RendererContext.h>
#include <scene/ComponentFunctionRegistry.h>
#include <renderer/Shader.h>

class UniformApplier : public ComponentFunctionRegistry<RendererContext&>
{
private:
	void PositionApplier(Entity entity, RendererContext& context);
	void RotationApplier(Entity entity, RendererContext& context);
	void ScaleApplier(Entity entity, RendererContext& context);
public:
	UniformApplier();
};