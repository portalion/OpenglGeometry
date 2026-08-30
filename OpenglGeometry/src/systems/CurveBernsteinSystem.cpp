#include "CurveBernsteinSystem.h"

#include <vector>

#include "archetypes/Archetypes.h"
#include "core/Globals.h"
#include "meshGenerators/MeshGenerators.h"
#include "scene/Components.h"
#include "scene/ObjectType.h"
#include "scene/Tags.h"

namespace
{
	constexpr float positionEpsilon = 1e-6f;
	constexpr float dragEpsilon = 1e-4f;

	constexpr float deBoorPerBernstein = 1.5f;

	bool IsC2Curve(Entity entity)
	{
		return entity.HasComponent<ObjectTypeComponent>()
			&& entity.GetComponent<ObjectTypeComponent>().type == ObjectType::BezierC2;
	}

	std::vector<Entity> DeBoorPoints(Entity curve)
	{
		std::vector<Entity> points;

		for (Entity point : curve.GetComponent<LineGenerationComponent>().controlPoints)
		{
			if (point.IsValid() && point.HasComponent<PositionComponent>())
			{
				points.push_back(point);
			}
		}

		return points;
	}

	std::vector<Algebra::Vector4> PositionsOf(const std::vector<Entity>& points)
	{
		std::vector<Algebra::Vector4> positions;
		positions.reserve(points.size());

		for (Entity point : points)
		{
			Algebra::Vector4 position = point.GetComponent<PositionComponent>().position;
			position.w = 1.f;
			positions.push_back(position);
		}

		return positions;
	}
}

CurveBernsteinSystem::CurveBernsteinSystem(Ref<Scene> scene)
	: m_Scene{ scene }
{
}

Entity CurveBernsteinSystem::CreateBernsteinPoint(Entity curve, const Algebra::Vector4& position)
{
	Entity point = m_Scene->CreateEntity();

	Archetypes::AddVirtualToEntity(point, curve);
	Archetypes::AddPointToEntity(point, position);
	point.AddComponent<ColorComponent>().color = Globals::bernsteinPointColor;

	auto& bernstein = point.AddComponent<BernsteinPointComponent>();
	bernstein.curve = curve;
	bernstein.lastComputed = position;

	point.AddTag<IsInvisibleTag>();

	return point;
}

void CurveBernsteinSystem::Resize(Entity curve, std::size_t wanted)
{
	auto& helpers = curve.GetComponent<CurveHelpersComponent>();

	if (!helpers.bernsteinPolyline.IsValid())
	{
		Entity polyline = m_Scene->CreateEntity();
		Archetypes::AddVirtualToEntity(polyline, curve);
		polyline.AddComponent<LineGenerationComponent>();
		polyline.AddComponent<ColorComponent>().color = Globals::bernsteinPointColor;
		polyline.AddTag<IsInvisibleTag>();
		polyline.AddTag<IsDirtyTag>();
		helpers.bernsteinPolyline = polyline;
	}

	while (helpers.bernsteinPoints.size() > wanted)
	{
		Entity extra = helpers.bernsteinPoints.back();
		helpers.bernsteinPoints.pop_back();

		if (extra.IsValid())
		{
			extra.RemoveTag<IsSelectedTag>();
			extra.AddTag<ToBeDestroyedTag>();
		}
	}

	while (helpers.bernsteinPoints.size() < wanted)
	{
		helpers.bernsteinPoints.push_back(
			CreateBernsteinPoint(curve, Algebra::Vector4(0.f, 0.f, 0.f, 1.f)));
	}

	auto& line = helpers.bernsteinPolyline.GetComponent<LineGenerationComponent>().controlPoints;
	line.assign(helpers.bernsteinPoints.begin(), helpers.bernsteinPoints.end());
	helpers.bernsteinPolyline.AddTag<IsDirtyTag>();
}

bool CurveBernsteinSystem::PushDraggedPointsBack(Entity curve, const std::vector<Entity>& deBoorPoints)
{
	auto& helpers = curve.GetComponent<CurveHelpersComponent>();
	bool moved = false;

	for (Entity point : helpers.bernsteinPoints)
	{
		if (!point.IsValid() || !point.HasComponent<BernsteinPointComponent>())
		{
			continue;
		}

		auto& bernstein = point.GetComponent<BernsteinPointComponent>();
		const Algebra::Vector4 current = point.GetComponent<PositionComponent>().position;

		Algebra::Vector4 delta = current - bernstein.lastComputed;
		delta.w = 0.f;

		if (delta.Length() < dragEpsilon || bernstein.deBoorIndex >= deBoorPoints.size())
		{
			continue;
		}

		Entity owner = deBoorPoints[bernstein.deBoorIndex];
		Algebra::Vector4 target = Algebra::Vector4(owner.GetComponent<PositionComponent>().position)
			+ delta * deBoorPerBernstein;
		target.w = 1.f;

		// Through the Observable, so the curve and everything else using this point rebuild.
		owner.GetComponent<PositionComponent>().position = target;
		moved = true;
	}

	return moved;
}

void CurveBernsteinSystem::Recompute(Entity curve)
{
	const std::vector<Entity> deBoorPoints = DeBoorPoints(curve);
	const std::vector<Algebra::Vector4> positions = PositionsOf(deBoorPoints);
	const std::vector<Algebra::Vector4> bernstein =
		MeshGenerator::BezierCurveC2::ToBernsteinPoints(positions);

	auto& helpers = curve.GetComponent<CurveHelpersComponent>();

	if (helpers.bernsteinPoints.size() != bernstein.size() || !helpers.bernsteinPolyline.IsValid())
	{
		Resize(curve, bernstein.size());
	}

	bool moved = false;

	for (std::size_t i = 0; i < helpers.bernsteinPoints.size() && i < bernstein.size(); i++)
	{
		Entity point = helpers.bernsteinPoints[i];
		if (!point.IsValid() || !point.HasComponent<BernsteinPointComponent>())
		{
			continue;
		}

		auto& bernsteinPoint = point.GetComponent<BernsteinPointComponent>();
		bernsteinPoint.deBoorIndex = MeshGenerator::BezierCurveC2::DeBoorIndexOf(i, positions.size());

		auto& position = point.GetComponent<PositionComponent>().position;
		if ((Algebra::Vector4(position) - bernstein[i]).Length() >= positionEpsilon)
		{
			position.value = bernstein[i];
			moved = true;
		}

		bernsteinPoint.lastComputed = bernstein[i];
	}

	if (moved && helpers.bernsteinPolyline.IsValid())
	{
		helpers.bernsteinPolyline.AddTag<IsDirtyTag>();
	}
}

void CurveBernsteinSystem::ApplyVisibility(Entity curve)
{
	auto& helpers = curve.GetComponent<CurveHelpersComponent>();
	const bool visible = helpers.showBernstein;

	const auto apply = [visible](Entity entity)
	{
		if (!entity.IsValid())
		{
			return;
		}

		if (visible && entity.HasComponent<IsInvisibleTag>())
		{
			entity.RemoveTag<IsInvisibleTag>();
		}
		else if (!visible && !entity.HasComponent<IsInvisibleTag>())
		{
			entity.RemoveTag<IsSelectedTag>();
			entity.AddTag<IsInvisibleTag>();
		}
	};

	for (Entity point : helpers.bernsteinPoints)
	{
		apply(point);
	}
	apply(helpers.bernsteinPolyline);
}

void CurveBernsteinSystem::Process()
{
	auto view = m_Scene->GetAllEntitiesWith<CurveHelpersComponent, LineGenerationComponent>();
	std::vector<Entity> curves(view.begin(), view.end());

	for (Entity curve : curves)
	{
		if (!curve.IsValid() || !IsC2Curve(curve))
		{
			continue;
		}

		if (curve.GetComponent<CurveHelpersComponent>().showBernstein)
		{
			PushDraggedPointsBack(curve, DeBoorPoints(curve));
		}

		Recompute(curve);
		ApplyVisibility(curve);
	}
}
