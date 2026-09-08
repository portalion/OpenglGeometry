#include "MeshGeneratingSystem.h"
#include "meshGenerators/MeshGenerators.h"
#include "scene/ObjectType.h"
#include "geometry/ParametricSurfaceFactory.h"
#include "geometry/IntersectionFinder.h"
#include "geometry/TrimMask.h"
#include "renderer/Texture2D.h"
#include "core/Globals.h"

std::vector<Algebra::Vector4> MeshGeneratingSystem::
CopyValidPointsToVector(std::list<Entity>& pointEntities)
{
	std::vector<Algebra::Vector4> positions;
	positions.reserve(pointEntities.size());

	for (auto it = pointEntities.begin(); it != pointEntities.end(); )
	{
		if (!it->IsValid() || !it->HasComponent<PositionComponent>())
		{
			it = pointEntities.erase(it);
			continue;
		}

		Algebra::Vector4 position = it->GetComponent<PositionComponent>().position;
		position.w = 1.f;
		positions.push_back(position);
		it++;
	}

	return positions;
}

std::vector<uint32_t> MeshGeneratingSystem::GenerateLineIndices(unsigned int verticesSize)
{
	std::vector<uint32_t> result;
	result.reserve(verticesSize);
	for (int i = 0; i < static_cast<int>(verticesSize); i++)
	{
		result.push_back(i);
	}
	return result;
}

void MeshGeneratingSystem::TorusGeneration()
{
	// position + (u, v) per vertex, so the torus fragment shader can sample a trim mask.
	BufferLayout torusLayout
	({
		{ ShaderDataType::Float4, "a_Position" },
		{ ShaderDataType::Float4, "a_uv" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, TorusGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		const auto& tgc = entity.GetComponent<TorusGenerationComponent>();

		auto generatedMesh = MeshGenerator::Torus::GenerateMesh(
			tgc.radius, tgc.tubeRadius, tgc.radialSegments, tgc.tubularSegments);

		std::vector<Algebra::Vector4> vertices;
		vertices.reserve(generatedMesh.vertices.size() * 2);
		for (std::size_t k = 0; k < generatedMesh.vertices.size(); k++)
		{
			const unsigned int i = static_cast<unsigned int>(k) / tgc.tubularSegments;
			const unsigned int j = static_cast<unsigned int>(k) % tgc.tubularSegments;
			vertices.push_back(generatedMesh.vertices[k]);
			vertices.push_back(Algebra::Vector4(
				static_cast<float>(i) / static_cast<float>(tgc.radialSegments),
				static_cast<float>(j) / static_cast<float>(tgc.tubularSegments),
				0.f, 0.f));
		}

		ModifyOrCreateMesh(entity, vertices, generatedMesh.indices,
			torusLayout, RenderingMode::Lines, { AvailableShaders::TorusSurface });
	}
}

void MeshGeneratingSystem::LineGeneration()
{
	for(Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, LineGenerationComponent>(Excluded<BezierLineGenerationComponent>()))
	{
		entity.RemoveTag<IsDirtyTag>();

		auto& lineComponent = entity.GetComponent<LineGenerationComponent>();

		std::vector<Algebra::Vector4> positions =
			CopyValidPointsToVector(lineComponent.controlPoints);

		auto generatedMesh = MeshGenerator::Polyline::GenerateMesh(positions);

		ModifyOrCreateMesh(entity, generatedMesh.vertices, generatedMesh.indices,
			generatedMesh.layout);
	}
}

void MeshGeneratingSystem::BezierLineGeneration()
{
	BufferLayout bezierShaderLayout
	({
		{ ShaderDataType::Float4, "a_Position" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, LineGenerationComponent, BezierLineGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		auto& controlPoints = entity.GetComponent<LineGenerationComponent>().controlPoints;
		const auto& generatorFunction = entity.GetComponent<BezierLineGenerationComponent>().generationFunction;

		std::vector<Algebra::Vector4> positions =
			CopyValidPointsToVector(controlPoints);

		auto vertices = generatorFunction(positions);
		auto indices = GenerateLineIndices(static_cast<unsigned int>(vertices.size()));

		ModifyOrCreateMesh(entity, vertices, indices,
			bezierShaderLayout, RenderingMode::Patches, { AvailableShaders::BezierCurveC0 });
	}
}

void MeshGeneratingSystem::BezierSurfaceGeneration()
{
	// Each vertex carries its control point plus (patchCol, patchRow, patchCols, patchRows)
	// so the tessellation shaders can turn a per-patch coordinate into a global (u, v) for
	// the trim mask.
	BufferLayout bezierShaderLayout
	({
		{ ShaderDataType::Float4, "a_Position" },
		{ ShaderDataType::Float4, "a_patchInfo" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, BezierSurfaceGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();
		std::vector<Algebra::Vector4> vertices;
		auto patches = entity.GetComponent<BezierSurfaceGenerationComponent>().bezierPatches;

		const bool isC2 = GetObjectType(entity) == ObjectType::BezierSurfaceC2;
		const int patchRows = static_cast<int>(patches.size());
		const int patchCols = patchRows > 0 ? static_cast<int>(patches[0].size()) : 0;

		uint32_t indice = 0;
		std::vector<uint32_t> indices;
		for (int pi = 0; pi < patchRows; pi++)
			for (int pj = 0; pj < patchCols; pj++)
			{
				Entity patch = patches[pi][pj];

				MeshGenerator::BezierSurfaceC2::PatchGrid controlPositions;
				for(int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++)
					{
						controlPositions[i][j] =
							patch.GetComponent<BezierPatchGenerationComponent>()
							.controlPoints[i][j].GetComponent<PositionComponent>()
							.position;
					}

				if (isC2)
				{
					controlPositions = MeshGenerator::BezierSurfaceC2::DeBoorToBernstein(controlPositions);
				}

				const Algebra::Vector4 patchInfo(
					static_cast<float>(pj), static_cast<float>(pi),
					static_cast<float>(patchCols), static_cast<float>(patchRows));

				for(int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++)
					{
						vertices.push_back(controlPositions[i][j]);
						vertices.push_back(patchInfo);
						indices.push_back(indice++);
					}
			}

		ModifyOrCreateMesh(entity, vertices, indices,
			bezierShaderLayout, RenderingMode::Patches,
			{ AvailableShaders::BezierSurfaceHorizontal, AvailableShaders::BezierSurfaceVertical });
	}

}

namespace
{
	bool IsGregoryValid(const GregoryPatchGenerationComponent& gregory)
	{
		for (const auto& side : gregory.holeSides)
		{
			for (Entity point : side.curve)
			{
				if (!point.IsValid() || !point.HasComponent<PositionComponent>()) return false;
			}
			for (Entity point : side.inner)
			{
				if (!point.IsValid() || !point.HasComponent<PositionComponent>()) return false;
			}
		}
		return true;
	}

	void RebuildGregorySubPatches(GregoryPatchGenerationComponent& gregory)
	{
		if (!IsGregoryValid(gregory))
		{
			return;
		}

		std::array<MeshGenerator::GregoryFill::HoleBoundarySide, 3> sides{};
		for (int k = 0; k < 3; k++)
		{
			for (int i = 0; i < 4; i++)
			{
				Algebra::Vector4 curve =
					gregory.holeSides[k].curve[i].GetComponent<PositionComponent>().position;
				Algebra::Vector4 inner =
					gregory.holeSides[k].inner[i].GetComponent<PositionComponent>().position;
				curve.w = 1.f;
				inner.w = 1.f;
				sides[k].curve[i] = curve;
				sides[k].inner[i] = inner;
			}
		}

		gregory.subPatches = MeshGenerator::GregoryFill::BuildTriangular(sides);
	}

	Algebra::Vector4 CubicPoint(const std::array<Algebra::Vector4, 4>& c, float t)
	{
		const float s = 1.f - t;
		return s * s * s * c[0] + 3.f * s * s * t * c[1] + 3.f * s * t * t * c[2] + t * t * t * c[3];
	}

	std::vector<Algebra::Vector4> GregoryTangentSegments(
		const GregoryPatchGenerationComponent& gregory)
	{
		std::vector<Algebra::Vector4> segments;

		constexpr std::array<float, 5> samples{
			0.1f, 0.3f, 0.5f, 0.7f, 0.9f
		};

		for (int k = 0; k < 3; ++k)
		{
			std::array<Algebra::Vector4, 4> boundary{};
			std::array<Algebra::Vector4, 4> inner{};

			for (int i = 0; i < 4; ++i)
			{
				auto temp = gregory.holeSides[k].curve[i];
				boundary[i] = temp.GetComponent<PositionComponent>().position;
				auto temp2 = gregory.holeSides[k].inner[i];

				inner[i] = temp2.GetComponent<PositionComponent>().position;
					
			}

			for (float t : samples)
			{
				const Algebra::Vector4 base =
					CubicPoint(boundary, t);

				const Algebra::Vector4 innerPoint =
					CubicPoint(inner, t);

				Algebra::Vector4 direction =
					3.f * (base - innerPoint);

				direction.w = 0.f;

				if (direction.Length() > 0.f)
				{
					Algebra::Vector4 tip =
						base + 0.5f * direction.Normalize();

					tip.w = 1.f;

					segments.push_back(base);
					segments.push_back(tip);
				}
			}
		}

		return segments;
	}
}

void MeshGeneratingSystem::GregoryPatchGeneration()
{
	BufferLayout layout
	({
		{ ShaderDataType::Float4, "a_Position" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, GregoryPatchGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		auto& gregory = entity.GetComponent<GregoryPatchGenerationComponent>();
		RebuildGregorySubPatches(gregory);

		std::vector<Algebra::Vector4> vertices;
		std::vector<uint32_t> indices;
		for (const auto& subPatch : gregory.subPatches)
		{
			for (const Algebra::Vector4& point : subPatch)
			{
				indices.push_back(static_cast<uint32_t>(vertices.size()));
				vertices.push_back(point);
			}
		}

		ModifyOrCreateMesh(entity, vertices, indices,
			layout, RenderingMode::Patches,
			{ AvailableShaders::GregoryPatchHorizontal, AvailableShaders::GregoryPatchVertical });

		if (entity.HasComponent<IsParentOfVirtualEntitiesComponent>())
		{
			for (Entity child : entity.GetComponent<IsParentOfVirtualEntitiesComponent>().virtualEntities)
			{
				if (!child.IsValid() || !child.HasComponent<GregoryTangentComponent>())
				{
					continue;
				}

				std::vector<Algebra::Vector4> tangentVertices =
					GregoryTangentSegments(gregory);
				std::vector<uint32_t> tangentIndices = GenerateLineIndices(
					static_cast<unsigned int>(tangentVertices.size()));

				ModifyOrCreateMesh(child, tangentVertices, tangentIndices, layout, RenderingMode::Lines);
			}
		}
	}
}

void MeshGeneratingSystem::IntersectionCurveGeneration()
{
	BufferLayout layout
	({
		{ ShaderDataType::Float4, "a_Position" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, IntersectionCurveComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		auto& data = entity.GetComponent<IntersectionCurveComponent>();

		const bool surfacesAlive = data.surfaceP.IsValid()
			&& (data.selfIntersection || data.surfaceQ.IsValid());

		if (surfacesAlive && data.retraceRequested)
		{
			data.retraceRequested = false;

			Geometry::IntersectionSettings settings;
			settings.stepLength = data.stepLength;
			settings.precision = data.precision;
			settings.useCursor = data.useCursor;
			settings.cursorPosition = data.cursorPosition;

			const Geometry::IntersectionData found = Geometry::FindIntersections(
				data.surfaceP, data.selfIntersection ? Entity{} : data.surfaceQ, settings);

			if (!found.points.empty())
			{
				data.points = found.points;
				data.paramsP = found.paramsP;
				data.paramsQ = found.paramsQ;
				data.componentEnds = found.componentEnds;
				data.closed = found.closed;

				for (Entity surface : { data.surfaceP, data.surfaceQ })
				{
					if (surface.IsValid() && surface.HasComponent<TrimmingComponent>())
					{
						surface.GetComponent<TrimmingComponent>().maskDirty = true;
					}
				}
			}
		}

		if (data.points.size() < 2)
		{
			continue;
		}

		// One GL_LINES list, but no segment bridging two traced components.
		std::vector<uint32_t> segmentStops = data.componentEnds;
		if (segmentStops.empty())
		{
			segmentStops.push_back(static_cast<uint32_t>(data.points.size()));
		}

		std::vector<uint32_t> indices;
		uint32_t start = 0;
		for (uint32_t stop : segmentStops)
		{
			for (uint32_t i = start; i + 1 < stop; i++)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
			}
			start = stop;
		}

		ModifyOrCreateMesh(entity, data.points, indices, layout);
	}
}

void MeshGeneratingSystem::TrimMaskGeneration()
{
	for (Entity surface : m_Scene->GetAllEntitiesWith<TrimmingComponent>())
	{
		auto& trimming = surface.GetComponent<TrimmingComponent>();

		std::erase_if(trimming.curves, [](Entity c)
			{ return !c.IsValid() || !c.HasComponent<IntersectionCurveComponent>(); });

		const bool activeAlive = trimming.activeCurve.IsValid()
			&& trimming.activeCurve.HasComponent<IntersectionCurveComponent>();
		if (!activeAlive)
		{
			trimming.activeCurve = trimming.curves.empty() ? Entity{} : trimming.curves.front();
			trimming.maskDirty = true;
		}

		if (!trimming.maskDirty)
		{
			continue;
		}
		trimming.maskDirty = false;

		if (!trimming.activeCurve.IsValid())
		{
			trimming.mask = nullptr;
			trimming.enabled = false;
			continue;
		}

		const auto& curve = trimming.activeCurve.GetComponent<IntersectionCurveComponent>();
		const bool asP = curve.surfaceP == surface;
		const std::vector<Algebra::Vector4>& params = asP ? curve.paramsP : curve.paramsQ;
		const bool wrapU = asP ? curve.wrappedPU : curve.wrappedQU;
		const bool wrapV = asP ? curve.wrappedPV : curve.wrappedQV;

		if (params.size() < 3)
		{
			trimming.mask = nullptr;
			continue;
		}

		Geometry::TrimMaskData mask = Geometry::BuildTrimMask(
			params, wrapU, wrapV, Globals::trimMaskResolution, curve.componentEnds);

		if (!trimming.mask
			|| trimming.mask->Width() != mask.width
			|| trimming.mask->Height() != mask.height)
		{
			trimming.mask = CreateRef<Texture2D>(mask.width, mask.height);
		}
		trimming.mask->Upload(mask.pixels);
	}
}

void MeshGeneratingSystem::SurfaceControlNetGeneration()
{
	BufferLayout layout
	({
		{ ShaderDataType::Float4, "a_Position" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, SurfaceControlNetComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		const auto& grid = entity.GetComponent<SurfaceControlNetComponent>().grid;

		std::vector<Algebra::Vector4> vertices;
		std::vector<uint32_t> indices;

		const int rows = static_cast<int>(grid.size());
		for (int i = 0; i < rows; i++)
		{
			const int cols = static_cast<int>(grid[i].size());
			for (int j = 0; j < cols; j++)
			{
				const uint32_t current = static_cast<uint32_t>(vertices.size());

				Entity point = grid[i][j];
				Algebra::Vector4 position(0.f, 0.f, 0.f, 1.f);
				if (point.IsValid() && point.HasComponent<PositionComponent>())
				{
					position = point.GetComponent<PositionComponent>().position;
				}
				position.w = 1.f;
				vertices.push_back(position);

				if (j + 1 < cols)
				{
					indices.push_back(current);
					indices.push_back(current + 1);
				}
				if (i + 1 < rows)
				{
					indices.push_back(current);
					indices.push_back(current + static_cast<uint32_t>(cols));
				}
			}
		}

		ModifyOrCreateMesh(entity, vertices, indices, layout);
	}
}

MeshGeneratingSystem::MeshGeneratingSystem(Ref<Scene> m_Scene)
	:m_Scene {m_Scene}
{
}

void MeshGeneratingSystem::Process()
{
	BezierLineGeneration();
	BezierSurfaceGeneration();
	GregoryPatchGeneration();
	IntersectionCurveGeneration();
	TrimMaskGeneration();
	SurfaceControlNetGeneration();
	LineGeneration();
	TorusGeneration();
}
