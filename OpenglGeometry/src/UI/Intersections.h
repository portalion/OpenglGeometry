#pragma once
#include <algorithm>
#include <vector>
#include "core/Base.h"
#include "core/Log.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "scene/ObjectType.h"
#include "archetypes/Archetypes.h"
#include "geometry/ParametricSurfaceFactory.h"
#include "geometry/IntersectionFinder.h"
#include "geometry/IntersectionSplit.h"

namespace GUI
{
	inline std::vector<Entity> SelectedIntersectableSurfaces(Ref<Scene> scene)
	{
		std::vector<Entity> surfaces;
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, ObjectTypeComponent>())
		{
			if (Geometry::IsParametricSurface(entity))
			{
				surfaces.push_back(entity);
			}
		}
		return surfaces;
	}

	inline bool CanCreateIntersection(Ref<Scene> scene)
	{
		const std::size_t count = SelectedIntersectableSurfaces(scene).size();
		return count == 1 || count == 2;
	}

	inline void RegisterCurveNotifications(Entity curve, Entity surface)
	{
		if (!surface.IsValid())
		{
			return;
		}

		const auto addNotifier = [&](Entity point)
		{
			if (!point.IsValid() || !point.HasComponent<NotificationComponent>())
			{
				return;
			}
			auto& notify = point.GetComponent<NotificationComponent>().entitiesToNotify;
			if (std::find(notify.begin(), notify.end(), curve) == notify.end())
			{
				notify.push_back(curve);
			}
		};

		if (surface.HasComponent<BezierSurfaceGenerationComponent>())
		{
			for (const auto& patchRow : surface.GetComponent<BezierSurfaceGenerationComponent>().bezierPatches)
			{
				for (Entity patch : patchRow)
				{
					if (!patch.IsValid() || !patch.HasComponent<BezierPatchGenerationComponent>())
					{
						continue;
					}
					for (const auto& row : patch.GetComponent<BezierPatchGenerationComponent>().controlPoints)
					{
						for (Entity point : row)
						{
							addNotifier(point);
						}
					}
				}
			}
		}
	}

	inline void RegisterCurveForTrimming(Entity curve, Entity surface)
	{
		if (!surface.IsValid()
			|| !(surface.HasComponent<BezierSurfaceGenerationComponent>()
				|| surface.HasComponent<TorusGenerationComponent>()))
		{
			return;
		}
		if (!surface.HasComponent<TrimmingComponent>())
		{
			surface.AddComponent<TrimmingComponent>();
		}
		auto& trimming = surface.GetComponent<TrimmingComponent>();
		if (std::find(trimming.curves.begin(), trimming.curves.end(), curve) == trimming.curves.end())
		{
			trimming.curves.push_back(curve);
		}
		if (!trimming.activeCurve.IsValid())
		{
			trimming.activeCurve = curve;
		}
		trimming.maskDirty = true;
	}

	inline void CreateIntersection(Ref<Scene> scene, Geometry::IntersectionSettings settings = {})
	{
		const std::vector<Entity> surfaces = SelectedIntersectableSurfaces(scene);
		if (surfaces.size() != 1 && surfaces.size() != 2)
		{
			Logger::Warning("Intersection: select one surface (self-intersection) or two");
			return;
		}

		const bool self = surfaces.size() == 1;
		Entity entityP = surfaces.front();
		Entity entityQ = self ? Entity{} : surfaces.back();

		Ref<Geometry::IParametricSurface> surfaceP = Geometry::MakeParametricSurface(entityP);
		Ref<Geometry::IParametricSurface> surfaceQ = self
			? surfaceP : Geometry::MakeParametricSurface(entityQ);

		if (!surfaceP || !surfaceQ)
		{
			Logger::Warning("Intersection: unsupported or incomplete surface");
			return;
		}

		if (settings.useCursor)
		{
			settings.cursorPosition = Archetypes::GetCursorPosition(scene.get());
		}

		const Geometry::IntersectionData found =
			Geometry::FindIntersections(entityP, entityQ, settings);

		if (found.points.empty())
		{
			Logger::Warning("Intersection: no intersection found");
			return;
		}

		const std::vector<Geometry::IntersectionData> pieces =
			Geometry::SplitSelfCrossings(found);

		IntersectionCurveComponent base;
		base.surfaceP = entityP;
		base.surfaceQ = self ? entityP : entityQ;
		base.selfIntersection = self;
		base.wrappedPU = surfaceP->WrappedU();
		base.wrappedPV = surfaceP->WrappedV();
		base.wrappedQU = surfaceQ->WrappedU();
		base.wrappedQV = surfaceQ->WrappedV();
		base.stepLength = settings.stepLength;
		base.precision = settings.precision;
		base.useCursor = settings.useCursor;
		base.cursorPosition = settings.cursorPosition;
		base.splitPiece = pieces.size() > 1;

		for (const Geometry::IntersectionData& piece : pieces)
		{
			IntersectionCurveComponent data = base;
			data.points = piece.points;
			data.paramsP = piece.paramsP;
			data.paramsQ = piece.paramsQ;
			data.componentEnds = piece.componentEnds;
			data.closed = piece.closed;

			Entity curve = Archetypes::CreateIntersectionCurve(scene.get(), data);
			RegisterCurveNotifications(curve, entityP);
			RegisterCurveForTrimming(curve, entityP);
			if (!self)
			{
				RegisterCurveNotifications(curve, entityQ);
				RegisterCurveForTrimming(curve, entityQ);
			}
		}

		if (pieces.size() > 1)
		{
			Logger::Info("Intersection: self-crossing curve split into {} loops ({} points)",
				pieces.size(), found.points.size());
		}
		else
		{
			Logger::Info("Intersection: curve added ({} points)", found.points.size());
		}
	}
}
