#include "UniformApplier.h"
#include <scene/Components.h>

void UniformApplier::PositionApplier(Entity entity, RendererContext& context)
{
	auto position = entity.GetComponent<PositionComponent>().position;
	context.Position = Algebra::Matrix4::Translation(position);
	
}

void UniformApplier::RotationApplier(Entity entity, RendererContext& context)
{
	auto rotation = entity.GetComponent<RotationComponent>().rotation;
	context.Rotation = rotation.ToMatrix();
}

void UniformApplier::ScaleApplier(Entity entity, RendererContext& context)
{
	auto scale = entity.GetComponent<ScaleComponent>().scale;
	context.Scale = Algebra::Matrix4::DiagonalScaling(scale.x, scale.y, scale.z);
}

UniformApplier::UniformApplier()
{
	Bind<PositionComponent>(&UniformApplier::PositionApplier);
	Bind<RotationComponent>(&UniformApplier::RotationApplier);
	Bind<ScaleComponent>(&UniformApplier::ScaleApplier);
}
