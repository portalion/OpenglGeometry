#pragma once
#include "SimpleArchetypeCreation.h"
#include "meshGenerators/MeshGenerators.h"

namespace Archetypes
{
	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddBezierC0ToEntity(Entity entity, Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		entity.AddTag<IsDirtyTag>();

		auto virtualPolyline = scene->CreateEntity();
		AddVirtualToEntity(virtualPolyline, entity);
		AddPolylineToEntity(virtualPolyline, pointsBegin, pointsEnd);

		AddLineToEntity(entity, pointsBegin, pointsEnd);
		auto& bezierComponent = entity.AddComponent<BezierLineGenerationComponent>();
		bezierComponent.generationFunction = MeshGenerator::BezierCurveC0::GenerateVertices;
		return entity;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity CreateBezierC0(Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		auto resultBezierC0 = scene->CreateEntity();

		AddShapeToEntity(resultBezierC0, "Bezier C0");
		AddBezierC0ToEntity(resultBezierC0, scene, pointsBegin, pointsEnd);

		return resultBezierC0;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity AddBezierC2ToEntity(Entity entity, Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		entity.AddTag<IsDirtyTag>();

		auto virtualPolyline = scene->CreateEntity();
		AddVirtualToEntity(virtualPolyline, entity);
		AddPolylineToEntity(virtualPolyline, pointsBegin, pointsEnd);

		AddLineToEntity(entity, pointsBegin, pointsEnd);
		auto& bezierComponent = entity.AddComponent<BezierLineGenerationComponent>();
		bezierComponent.generationFunction = MeshGenerator::BezierCurveC2::GenerateVertices;
		return entity;
	}

	template<std::forward_iterator Iter>
		requires std::same_as<std::iter_value_t<Iter>, Entity>
	inline Entity CreateBezierC2(Scene* scene, Iter pointsBegin, const Iter& pointsEnd)
	{
		auto resultBezierC2 = scene->CreateEntity();

		AddShapeToEntity(resultBezierC2, "Bezier C2");
		AddBezierC2ToEntity(resultBezierC2, scene, pointsBegin, pointsEnd);

		return resultBezierC2;
	}
}
