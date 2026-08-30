#pragma once
#include "SimpleArchetypeCreation.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace Archetypes
{
	const unsigned int CONTROL_PONTS_PER_EDGE = 4;

	struct BezierSurfaceCreationParameters
	{
		bool isCylinder = false;
		float sizeX = 4;
		float sizeY = 4;
		unsigned int numberOfXPatches = 1;
		unsigned int numberOfYPatches = 1;
		Algebra::Vector4 startingPosition;
	};

	struct SurfaceDegree
	{
		unsigned int stride;
		unsigned int seam;
	};

	inline constexpr SurfaceDegree SurfaceDegreeC0{ 3u, 1u };
	inline constexpr SurfaceDegree SurfaceDegreeC2{ 1u, 3u };

	inline Entity CreateVirtualPatch(Scene* scene, Entity parentEntity)
	{
		auto entity = scene->CreateEntity();

		entity.AddComponent<BezierPatchGenerationComponent>();
		AddVirtualToEntity(entity, parentEntity);

		return entity;
	}

	inline Entity AddSurfaceControlNet(Scene* scene, Entity surface,
		const std::vector<std::vector<Entity>>& grid)
	{
		auto net = scene->CreateEntity();
		AddVirtualToEntity(net, surface);
		net.AddComponent<SurfaceControlNetComponent>().grid = grid;
		net.AddTag<IsDirtyTag>();

		for (const auto& column : grid)
		{
			for (Entity point : column)
			{
				if (!point.IsValid() || !point.HasComponent<NotificationComponent>())
				{
					continue;
				}

				auto& notify = point.GetComponent<NotificationComponent>().entitiesToNotify;
				if (std::find(notify.begin(), notify.end(), net) == notify.end())
				{
					notify.push_back(net);
				}
			}
		}

		return net;
	}

#pragma region Utils
	inline void AssignPointsToPatch(
		Entity patch,
		BezierPatchGenerationComponent& patchComponent,
		const std::vector<Entity>& points)
	{
		auto& patchPoints = patchComponent.controlPoints;
		for (int x = 0; x < CONTROL_PONTS_PER_EDGE; x++)
		{
			for (int y = 0; y < CONTROL_PONTS_PER_EDGE; y++)
			{
				auto point = points[x * CONTROL_PONTS_PER_EDGE + y];
				if (point.HasComponent<NotificationComponent>())
				{
					point.GetComponent<NotificationComponent>()
						.entitiesToNotify.push_back(patch);
				}
				patchPoints[x][y] = point;
			}
		}
	}

	inline std::vector<Entity> CreateLinearVectorFrom2D(size_t startX, size_t startY,
		const std::vector<std::vector<Entity>>& points2D)
	{
		std::vector<Entity> result;
		for (int x = 0; x < CONTROL_PONTS_PER_EDGE; x++)
		{
			for (int y = 0; y < CONTROL_PONTS_PER_EDGE; y++)
			{
				result.push_back(points2D[x + startX][y + startY]);
			}
		}
		return result;
	}
#pragma endregion
#pragma region Patches
	inline std::pair<unsigned int, unsigned int>
		CalculateNumberOfPointsForSurface(BezierSurfaceCreationParameters params, SurfaceDegree degree)
	{
		const unsigned int overlap = CONTROL_PONTS_PER_EDGE - degree.stride;
		const unsigned int numberOfPointsX = degree.stride * params.numberOfXPatches + overlap;
		const unsigned int numberOfPointsY = degree.stride * params.numberOfYPatches + overlap;
		return { numberOfPointsX, numberOfPointsY };
	}

	inline std::pair<unsigned int, unsigned int>
		CalculateNumberOfPointsForC0Surface(BezierSurfaceCreationParameters params)
	{
		return CalculateNumberOfPointsForSurface(params, SurfaceDegreeC0);
	}

	inline void FillBezierComponent(Scene* scene, Entity surface,
		BezierSurfaceGenerationComponent& result,
		BezierSurfaceCreationParameters bezierParams,
		const std::vector<std::vector<Entity>>& points,
		SurfaceDegree degree = SurfaceDegreeC0)
	{
		result.bezierPatches =
			std::vector<std::vector<Entity>>(bezierParams.numberOfXPatches,
				std::vector<Entity>(bezierParams.numberOfYPatches));

		for (unsigned int i = 0; i < bezierParams.numberOfXPatches; i++)
		{
			for (unsigned int j = 0; j < bezierParams.numberOfYPatches; j++)
			{
				auto patch = CreateVirtualPatch(scene, surface);
				result.bezierPatches[i][j] = patch;
				auto& patchComponent = patch.GetComponent<BezierPatchGenerationComponent>();
				auto pointsForPatch = CreateLinearVectorFrom2D(
					i * degree.stride, j * degree.stride, points);

				AssignPointsToPatch(patch, patchComponent, pointsForPatch);
			}
		}
	}

	inline std::vector<std::vector<Entity>>GenerateRectangularGridOfPoints(Scene* scene, Entity entity,
		BezierSurfaceCreationParameters params, std::pair<unsigned int, unsigned int> numberOfPoints,
		bool createVirtual, SurfaceDegree degree = SurfaceDegreeC0)
	{
		auto [numberOfPointsX, numberOfPointsY] = numberOfPoints;

		std::vector<std::vector<Entity>> result(numberOfPointsX, std::vector<Entity>(numberOfPointsY));

		const bool insetSurface = degree.stride == 1;
		const float spanX = static_cast<float>(insetSurface ? params.numberOfXPatches : numberOfPointsX - 1);
		const float spanY = static_cast<float>(insetSurface ? params.numberOfYPatches : numberOfPointsY - 1);
		const float sizeXPerPoint = params.sizeX / spanX;
		const float sizeYPerPoint = params.sizeY / spanY;
		const int indexOffset = insetSurface ? 1 : 0;

		const auto startingPosition = params.startingPosition;

		for (unsigned int i = 0; i < numberOfPointsX; i++)
		{
			for (unsigned int j = 0; j < numberOfPointsY; j++)
			{
				Algebra::Vector4 offset = Algebra::Vector4(
					(static_cast<int>(i) - indexOffset) * sizeXPerPoint, 0.f,
					(static_cast<int>(j) - indexOffset) * sizeYPerPoint);
				Entity point;
				if (!createVirtual)
					point = CreatePoint(scene, startingPosition + offset);
				else
				{
					point = scene->CreateEntity();
					AddVirtualToEntity(point, entity);
					AddPointToEntity(point, startingPosition + offset);
				}
				result[i][j] = point;
			}
		}

		return result;
	}

	inline std::vector<std::vector<Entity>> GenerateCylindricalGridOfPoints(Scene* scene, Entity entity,
		BezierSurfaceCreationParameters params, std::pair<unsigned int, unsigned int> numberOfPoints,
		bool createVirtual, SurfaceDegree degree = SurfaceDegreeC0)
	{
		auto [numberOfPointsX, numberOfPointsY] = numberOfPoints;

		const unsigned int distinctColumns = numberOfPointsX - degree.seam;
		const float anglePerPoint =
			2.f * std::numbers::pi_v<float> / static_cast<float>(distinctColumns);

		const bool insetSurface = degree.stride == 1;
		const float radius = insetSurface
			? params.sizeX * 3.f / (2.f + std::cos(anglePerPoint))
			: params.sizeX;
		const float spanY = static_cast<float>(insetSurface ? params.numberOfYPatches : numberOfPointsY - 1);
		const float heightPerPoint = params.sizeY / spanY;
		const int heightIndexOffset = insetSurface ? 1 : 0;

		std::vector<std::vector<Entity>> result(numberOfPointsX, std::vector<Entity>(numberOfPointsY));
		Algebra::Vector4 startingPosition = params.startingPosition;

		for(unsigned int i = 0; i < distinctColumns; i++)
		{
			for(unsigned int j = 0; j < numberOfPointsY; j++)
			{
				Algebra::Vector4 heightOffset =
					Algebra::Vector4(0.f, (static_cast<int>(j) - heightIndexOffset) * heightPerPoint, 0.f);
				Algebra::Vector4 radiusOffset =
					Algebra::Matrix4::RotationY(anglePerPoint * i) *
					Algebra::Vector4(radius, 0.f, 0.f);

				Entity point;
				if(!createVirtual)
					point = CreatePoint(scene, startingPosition + heightOffset + radiusOffset);
				else
				{
					point = scene->CreateEntity();
					AddVirtualToEntity(point, entity);
					AddPointToEntity(point, startingPosition + heightOffset + radiusOffset);
				}
				result[i][j] = point;
			}
		}

		for(unsigned int t = 0; t < degree.seam; t++)
		{
			for(unsigned int j = 0; j < numberOfPointsY; j++)
			{
				result[distinctColumns + t][j] = result[t][j];
			}
		}

		return result;
	}

#pragma endregion
	inline Entity AddBezierSurfaceToEntity(Entity entity, Scene* scene, BezierSurfaceCreationParameters bezierParams,
		SurfaceDegree degree = SurfaceDegreeC0, bool createVirtual = false)
	{
		entity.AddTag<IsDirtyTag>();
		auto& bezierComponent = entity.AddComponent<BezierSurfaceGenerationComponent>();

		auto numberOfPoints = CalculateNumberOfPointsForSurface(bezierParams, degree);

		std::vector<std::vector<Entity>> points;
		if (bezierParams.isCylinder)
			points = GenerateCylindricalGridOfPoints(scene, entity, bezierParams, numberOfPoints, createVirtual, degree);
		else
			points = GenerateRectangularGridOfPoints(scene, entity, bezierParams, numberOfPoints, createVirtual, degree);

		FillBezierComponent(scene, entity, bezierComponent, bezierParams, points, degree);
		AddSurfaceControlNet(scene, entity, points);

		return entity;
	}

	inline Entity CreateBezierSurface(Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		auto resultBezierSurface = scene->CreateEntity();

		AddShapeToEntity(resultBezierSurface, ObjectType::BezierSurfaceC0);
		AddBezierSurfaceToEntity(resultBezierSurface, scene, bezierParams, SurfaceDegreeC0);

		return resultBezierSurface;
	}

	inline Entity CreateBezierSurfaceC2(Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		auto resultBezierSurface = scene->CreateEntity();

		AddShapeToEntity(resultBezierSurface, ObjectType::BezierSurfaceC2);
		AddBezierSurfaceToEntity(resultBezierSurface, scene, bezierParams, SurfaceDegreeC2);

		return resultBezierSurface;
	}

	inline Entity CreateBezierSurfaceFromControlGrid(Scene* scene,
		const std::vector<std::vector<Entity>>& grid, int samplesU, int samplesV, bool isC2)
	{
		const SurfaceDegree degree = isC2 ? SurfaceDegreeC2 : SurfaceDegreeC0;
		const unsigned int overlap = CONTROL_PONTS_PER_EDGE - degree.stride;

		auto surface = scene->CreateEntity();
		AddShapeToEntity(surface, isC2 ? ObjectType::BezierSurfaceC2 : ObjectType::BezierSurfaceC0);
		surface.AddTag<IsDirtyTag>();

		auto& bezierComponent = surface.AddComponent<BezierSurfaceGenerationComponent>();
		bezierComponent.samplesU = samplesU;
		bezierComponent.samplesV = samplesV;

		BezierSurfaceCreationParameters params;
		params.numberOfXPatches = (static_cast<unsigned int>(grid.size()) - overlap) / degree.stride;
		params.numberOfYPatches = (static_cast<unsigned int>(grid[0].size()) - overlap) / degree.stride;

		FillBezierComponent(scene, surface, bezierComponent, params, grid, degree);
		AddSurfaceControlNet(scene, surface, grid);

		return surface;
	}
}
