#include "UniformApplier.h"
#include <scene/Components.h>

void UniformApplier::PositionApplier(Entity entity, EntityContext& context)
{
	auto position = entity.GetComponent<PositionComponent>().position;
	context.Position = Algebra::Matrix4::Translation(position);
	
}

void UniformApplier::RotationApplier(Entity entity, EntityContext& context)
{
	auto rotation = entity.GetComponent<RotationComponent>().rotation;
	context.Rotation = rotation.ToMatrix();
}

void UniformApplier::ScaleApplier(Entity entity, EntityContext& context)
{
	auto scale = entity.GetComponent<ScaleComponent>().scale;
	context.Scale = Algebra::Matrix4::DiagonalScaling(scale.x, scale.y, scale.z);
}

void UniformApplier::ColorApplier(Entity entity, EntityContext& context)
{
	context.Color = entity.GetComponent<ColorComponent>().color;
}

void UniformApplier::SamplesApplier(Entity entity, EntityContext& context)
{
	const auto& surface = entity.GetComponent<BezierSurfaceGenerationComponent>();
	context.SamplesU = surface.samplesU;
	context.SamplesV = surface.samplesV;
}

UniformApplier::UniformApplier()
{
	Bind<PositionComponent>(&UniformApplier::PositionApplier);
	Bind<RotationComponent>(&UniformApplier::RotationApplier);
	Bind<ScaleComponent>(&UniformApplier::ScaleApplier);
	Bind<ColorComponent>(&UniformApplier::ColorApplier);
	Bind<BezierSurfaceGenerationComponent>(&UniformApplier::SamplesApplier);
}
