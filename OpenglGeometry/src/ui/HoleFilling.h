#pragma once
#include <array>
#include <optional>
#include <vector>
#include "core/Base.h"
#include "core/Log.h"
#include "core/Globals.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "scene/ObjectType.h"
#include "archetypes/Archetypes.h"
#include "meshGenerators/GregoryFillGenerator.h"

namespace GUI
{
	inline Algebra::Vector4 GetControlPointPosition(Entity point)
	{
		return point.GetComponent<PositionComponent>().position;
	}

	inline std::vector<Algebra::Vector4> GetControlPointPositions(const std::vector<Entity>& points)
	{
		std::vector<Algebra::Vector4> result;
		result.reserve(points.size());
		for (Entity point : points)
		{
			result.push_back(GetControlPointPosition(point));
		}
		return result;
	}

	struct Edge
	{
		std::vector<Entity> PointEntities;
		std::vector<Entity> InnerPointEntities;
		BezierPatchGenerationComponent& Patch;

		bool operator==(const Edge& r) const
		{
			const std::vector<Algebra::Vector4> points = GetControlPointPositions(PointEntities);
			const std::vector<Algebra::Vector4> other = GetControlPointPositions(r.PointEntities);
			for (int i = 0; i < points.size(); i++)
			{
				bool isGood = false;
				for (int j = 0; j < other.size(); j++)
				{
					if (other[j] == points[i]) isGood = true;
				}
				if (!isGood) return false;
			}
			return true;
		}
	};

	inline std::array<Edge, 4> GetPatchEdges(Entity patch)
	{
		auto& patchComponent = patch.GetComponent<BezierPatchGenerationComponent>();
		auto& controlPoints = patchComponent.controlPoints;

		Edge top{ {}, {}, patchComponent };
		Edge bottom{ {}, {}, patchComponent };
		Edge left{ {}, {}, patchComponent };
		Edge right{ {}, {}, patchComponent };

		for (int i = 0; i < 4; i++)
		{
			top.PointEntities.push_back(controlPoints[0][i]);
			top.InnerPointEntities.push_back(controlPoints[1][i]);

			bottom.PointEntities.push_back(controlPoints[3][i]);
			bottom.InnerPointEntities.push_back(controlPoints[2][i]);

			left.PointEntities.push_back(controlPoints[i][0]);
			left.InnerPointEntities.push_back(controlPoints[i][1]);

			right.PointEntities.push_back(controlPoints[i][3]);
			right.InnerPointEntities.push_back(controlPoints[i][2]);
		}

		return { top, bottom, left, right };
	}

	inline void ClearDebugVisuals(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<DebugTag>())
		{
			entity.AddTag<ToBeDestroyedTag>();
		}
	}

	inline void CreateDebugEdge(Ref<Scene> scene, const Edge& edge,
		const Algebra::Vector4& color = Globals::debugEdgeColor)
	{
		if (edge.PointEntities.size() < 2) return;

		std::vector<Entity> points = edge.PointEntities;
		Entity debugLine = Archetypes::CreatePolyline(scene.get(), points.begin(), points.end());
		debugLine.AddTag<DebugTag>();
		debugLine.AddComponent<ColorComponent>().color = color;
	}

	inline std::optional<Algebra::Vector4> SharedCorner(const Edge& a, const Edge& b)
	{
		const std::vector<Algebra::Vector4> aPoints = GetControlPointPositions(a.PointEntities);
		const std::vector<Algebra::Vector4> bPoints = GetControlPointPositions(b.PointEntities);

		if (aPoints.front() == bPoints.front() || aPoints.front() == bPoints.back())
		{
			return aPoints.front();
		}
		if (aPoints.back() == bPoints.front() || aPoints.back() == bPoints.back())
		{
			return aPoints.back();
		}
		return std::nullopt;
	}

	struct EdgeCycle
	{
		Edge First;
		Edge Second;
		Edge Third;
	};

	inline std::vector<EdgeCycle> FindEdgeCycles(const std::vector<Edge>& edges)
	{
		std::vector<EdgeCycle> cycles;

		for (size_t i = 0; i < edges.size(); i++)
		{
			for (size_t j = i + 1; j < edges.size(); j++)
			{
				std::optional<Algebra::Vector4> cornerIJ = SharedCorner(edges[i], edges[j]);
				if (!cornerIJ) continue;

				for (size_t k = j + 1; k < edges.size(); k++)
				{
					std::optional<Algebra::Vector4> cornerJK = SharedCorner(edges[j], edges[k]);
					if (!cornerJK) continue;

					std::optional<Algebra::Vector4> cornerKI = SharedCorner(edges[k], edges[i]);
					if (!cornerKI) continue;

					if (*cornerIJ == *cornerJK || *cornerJK == *cornerKI || *cornerKI == *cornerIJ) continue;

					cycles.push_back({ edges[i], edges[j], edges[k] });
				}
			}
		}

		return cycles;
	}

	inline int FindContainingCycle(const Edge& edge, const std::vector<EdgeCycle>& cycles)
	{
		for (int i = 0; i < static_cast<int>(cycles.size()); i++)
		{
			if (edge == cycles[i].First || edge == cycles[i].Second || edge == cycles[i].Third)
			{
				return i;
			}
		}
		return -1;
	}

	inline Algebra::Vector4 DebugCycleColor(int index)
	{
		static const std::array<Algebra::Vector4, 6> palette
		{
			Algebra::Vector4(1.f, 0.f, 0.f, 1.f),
			Algebra::Vector4(0.f, 1.f, 0.f, 1.f),
			Algebra::Vector4(0.f, 0.6f, 1.f, 1.f),
			Algebra::Vector4(1.f, 0.6f, 0.f, 1.f),
			Algebra::Vector4(0.6f, 0.f, 1.f, 1.f),
			Algebra::Vector4(1.f, 1.f, 0.f, 1.f),
		};

		return palette[index % palette.size()];
	}

	inline std::vector<Edge> CollectOpenBoundaryEdges(Ref<Scene> scene)
	{
		std::vector<Edge> allEdges;

		for (Entity surface : scene->GetAllEntitiesWith<BezierSurfaceGenerationComponent, IsSelectedTag>())
		{
			if (!surface.IsValid() || GetObjectType(surface) != ObjectType::BezierSurfaceC0) continue;

			for (const auto& patchRow : surface.GetComponent<BezierSurfaceGenerationComponent>().bezierPatches)
			{
				for (Entity patch : patchRow)
				{
					if (!patch.IsValid() || !patch.HasComponent<BezierPatchGenerationComponent>()) continue;

					for (const Edge& edge : GetPatchEdges(patch))
					{
						allEdges.push_back(edge);
					}
				}
			}
		}

		std::vector<Edge> edges;

		for (const Edge& edge : allEdges)
		{
			int matches = 0;
			for (const Edge& other : allEdges)
			{
				if (edge == other) matches++;
			}

			if (matches == 1)
			{
				edges.push_back(edge);
			}
		}

		return edges;
	}

	inline void FindHoles(Ref<Scene> scene)
	{
		ClearDebugVisuals(scene);

		const std::vector<Edge> edges = CollectOpenBoundaryEdges(scene);
		std::vector<EdgeCycle> holes = FindEdgeCycles(edges);

		for (const Edge& edge : edges)
		{
			int cycleIndex = FindContainingCycle(edge, holes);
			CreateDebugEdge(scene, edge, cycleIndex >= 0 ? DebugCycleColor(cycleIndex) : Globals::debugEdgeColor);
		}

		Logger::Info("There are {} edges and {} 3-edge holes", edges.size(), holes.size());
	}

	inline std::optional<std::array<Algebra::Vector4, 4>> OrientedRow(const Edge& edge,
		const Algebra::Vector4& start, bool takeInner)
	{
		const std::vector<Algebra::Vector4> points = GetControlPointPositions(edge.PointEntities);
		const std::vector<Algebra::Vector4> source = takeInner
			? GetControlPointPositions(edge.InnerPointEntities) : points;
		if (points.size() != 4 || source.size() != 4)
		{
			return std::nullopt;
		}

		const bool forward = points.front() == start;
		const bool backward = points.back() == start;
		if (!forward && !backward)
		{
			return std::nullopt;
		}

		std::array<Algebra::Vector4, 4> result{};
		for (int i = 0; i < 4; i++)
		{
			result[i] = forward ? source[i] : source[3 - i];
		}
		return result;
	}

	inline std::optional<std::array<Entity, 4>> OrientedRowEntities(const Edge& edge,
		const Algebra::Vector4& start, bool takeInner)
	{
		if (edge.PointEntities.size() != 4 || edge.InnerPointEntities.size() != 4)
		{
			return std::nullopt;
		}

		const std::vector<Algebra::Vector4> points = GetControlPointPositions(edge.PointEntities);
		const bool forward = points.front() == start;
		const bool backward = points.back() == start;
		if (!forward && !backward)
		{
			return std::nullopt;
		}

		const auto& source = takeInner ? edge.InnerPointEntities : edge.PointEntities;
		std::array<Entity, 4> result{};
		for (int i = 0; i < 4; i++)
		{
			result[i] = forward ? source[i] : source[3 - i];
		}
		return result;
	}

	struct OrderedHole
	{
		std::array<MeshGenerator::GregoryFill::HoleBoundarySide, 3> sides;
		std::array<GregoryHoleSide, 3> refs;
	};

	inline std::optional<OrderedHole> OrderTriangularHole(const EdgeCycle& cycle)
	{
		const std::array<const Edge*, 3> edges{ &cycle.First, &cycle.Second, &cycle.Third };
		const std::array<std::vector<Algebra::Vector4>, 3> points
		{
			GetControlPointPositions(edges[0]->PointEntities),
			GetControlPointPositions(edges[1]->PointEntities),
			GetControlPointPositions(edges[2]->PointEntities),
		};
		for (int c = 0; c < 3; c++)
		{
			if (points[c].size() != 4 || edges[c]->InnerPointEntities.size() != 4)
			{
				return std::nullopt;
			}
		}

		for (int orientation = 0; orientation < 2; orientation++)
		{
			std::array<bool, 3> used{ false, false, false };
			OrderedHole ordered{};

			Algebra::Vector4 cursor = orientation == 0 ? points[0].front() : points[0].back();
			const Algebra::Vector4 loopStart = cursor;
			bool ok = true;

			for (int k = 0; k < 3; k++)
			{
				int pick = -1;
				for (int c = 0; c < 3; c++)
				{
					if (used[c]) continue;
					if (points[c].front() == cursor || points[c].back() == cursor)
					{
						pick = c;
						break;
					}
				}

				if (pick < 0) { ok = false; break; }
				used[pick] = true;

				auto curve = OrientedRow(*edges[pick], cursor, false);
				auto inner = OrientedRow(*edges[pick], cursor, true);
				auto curveRefs = OrientedRowEntities(*edges[pick], cursor, false);
				auto innerRefs = OrientedRowEntities(*edges[pick], cursor, true);
				if (!curve || !inner || !curveRefs || !innerRefs) { ok = false; break; }

				ordered.sides[k].curve = *curve;
				ordered.sides[k].inner = *inner;
				ordered.refs[k].curve = *curveRefs;
				ordered.refs[k].inner = *innerRefs;
				cursor = ordered.sides[k].curve[3];
			}

			if (ok && cursor == loopStart)
			{
				return ordered;
			}
		}

		return std::nullopt;
	}

	inline void FillHoles(Ref<Scene> scene)
	{
		const std::vector<Edge> edges = CollectOpenBoundaryEdges(scene);
		const std::vector<EdgeCycle> holes = FindEdgeCycles(edges);

		int filled = 0;
		for (const EdgeCycle& hole : holes)
		{
			auto ordered = OrderTriangularHole(hole);
			if (!ordered)
			{
				Logger::Warning("Fill holes: could not order a 3-edge hole - skipping");
				continue;
			}

			const auto subPatches = MeshGenerator::GregoryFill::BuildTriangular(ordered->sides);
			Archetypes::CreateGregoryPatch(scene.get(), subPatches, ordered->refs);
			filled++;
		}

		if (filled > 0)
		{
			Logger::Info("Filled {} triangular hole(s) with Gregory patches", filled);
		}
		else
		{
			Logger::Warning("Fill holes: no triangular hole found on the selected C0 surface(s)");
		}
	}
}
