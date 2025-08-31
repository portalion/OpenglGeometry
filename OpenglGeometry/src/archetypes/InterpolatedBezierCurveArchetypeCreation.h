#pragma once
#include "SimpleArchetypeCreation.h"
#include "meshGenerators/MeshGenerators.h"

namespace Archetypes
{
	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddInterpolatedBezierToEntity(Entity entity, Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		entity.AddTag<IsDirtyTag>();

		AddLineToEntity(entity, pointsBegin, pointsEnd);
		auto& bezierComponent = entity.AddComponent<BezierLineGenerationComponent>();
		bezierComponent.generationFunction =
			[](const std::vector<Algebra::Vector4>& controlPoints)
		{
			return MeshGenerator::InterpolatedBezierCurve::GenerateMesh(controlPoints).vertices;
		};

		return entity;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity CreateInterpolatedBezier(Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		auto resultBezierC0 = scene->CreateEntity();

		AddShapeToEntity(resultBezierC0, "Interpolated Bezier");
		AddInterpolatedBezierToEntity(resultBezierC0, scene, pointsBegin, pointsEnd);

		return resultBezierC0;
	}
}
