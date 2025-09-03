#pragma once
#include "SimpleArchetypeCreation.h"
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
	};

	inline Entity CreateVirtualPatch(Scene* scene, Entity parentEntity)
	{
		auto entity = scene->CreateEntity();

		entity.AddComponent<BezierPatchGenerationComponent>();
		AddVirtualToEntity(entity, parentEntity);

		return entity;
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
#pragma region C0 Patches
	inline std::pair<unsigned int, unsigned int>
		CalculateNumberOfPointsForC0Surface(BezierSurfaceCreationParameters params)
	{
		const unsigned int numberOfPointsX =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfXPatches + 1;
		const unsigned int numberOfPointsY =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfYPatches + 1;
		return { numberOfPointsX, numberOfPointsY };
	}

	inline void FillBezierComponent(Scene* scene, Entity surface,
		BezierSurfaceGenerationComponent& result,
		BezierSurfaceCreationParameters bezierParams,
		const std::vector<std::vector<Entity>>& points)
	{
		result.bezierPatches =
			std::vector<std::vector<Entity>>(bezierParams.numberOfXPatches,
				std::vector<Entity>(bezierParams.numberOfYPatches));

		for (int i = 0; i < bezierParams.numberOfXPatches; i++)
		{
			for (int j = 0; j < bezierParams.numberOfYPatches; j++)
			{
				auto patch = CreateVirtualPatch(scene, surface);
				result.bezierPatches[i][j] = patch;
				auto& patchComponent = patch.GetComponent<BezierPatchGenerationComponent>();
				auto pointsForPatch = CreateLinearVectorFrom2D(
					i * (CONTROL_PONTS_PER_EDGE - 1), j * (CONTROL_PONTS_PER_EDGE - 1), points);

				AssignPointsToPatch(patch, patchComponent, pointsForPatch);
			}
		}
	}

	inline std::vector<std::vector<Entity>>GenerateRectangularGridOfPoints(Scene* scene, 
		BezierSurfaceCreationParameters params, std::pair<unsigned int, unsigned int> numberOfPoints)
	{
		auto [numberOfPointsX, numberOfPointsY] = numberOfPoints;

		std::vector<std::vector<Entity>> result(numberOfPointsX, std::vector<Entity>(numberOfPointsY));

		const float sizeXPerPoint = params.sizeX / (numberOfPointsX - 1);
		const float sizeYPerPoint = params.sizeY / (numberOfPointsY - 1);

		for (int i = 0; i < numberOfPointsX; i++)
		{
			for (int j = 0; j < numberOfPointsY; j++)
			{
				auto point = CreatePoint(scene,
					{
						i * sizeXPerPoint,
						j * sizeYPerPoint,
						0.f
					});
				result[i][j] = point;
			}
		}

		return result;
	}

	inline std::vector<std::vector<Entity>> GenerateCylindricalGridOfPoints(Scene* scene, 
		BezierSurfaceCreationParameters params, std::pair<unsigned int, unsigned int> numberOfPoints)
	{
		auto [numberOfPointsX, numberOfPointsY] = numberOfPoints;
		
		const float heightPerPoint = params.sizeY / (params.numberOfYPatches * 3);
		const float anglePerPoint = 
			2.f * std::numbers::pi_v<float> / static_cast<float>(numberOfPointsX - 1);

		std::vector<std::vector<Entity>> result(numberOfPointsX, std::vector<Entity>(numberOfPointsY));
		Algebra::Vector4 startingPosition;

		for(int i = 0; i < numberOfPointsX - 1; i++)
		{
			for(int j = 0; j < numberOfPointsY; j++)
			{
				Algebra::Vector4 heightOffset = 
					Algebra::Vector4(0.f, 0.f, j * heightPerPoint);
				Algebra::Vector4 radiusOffset = 
					Algebra::Matrix4::RotationZ(anglePerPoint * i) * 
					Algebra::Vector4(params.sizeX, 0.f, 0.f);

				auto point = CreatePoint(scene, startingPosition + heightOffset + radiusOffset);
				result[i][j] = point;
			}
		}

		for(int i = 0; i < numberOfPointsY; i++)
		{
			result[numberOfPointsX - 1][i] = result[0][i];
		}

		return result;
	}

#pragma endregion
	inline Entity AddBezierSurfaceToEntity(Entity entity, Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		entity.AddTag<IsDirtyTag>();
		auto& bezierComponent = entity.AddComponent<BezierSurfaceGenerationComponent>();

		auto numberOfPoints = CalculateNumberOfPointsForC0Surface(bezierParams);

		std::vector<std::vector<Entity>> points;
		if (bezierParams.isCylinder)
			points = GenerateCylindricalGridOfPoints(scene, bezierParams, numberOfPoints);
		else
			points = GenerateRectangularGridOfPoints(scene, bezierParams, numberOfPoints);

		FillBezierComponent(scene, entity, bezierComponent, bezierParams, points);

		return entity;
	}

	inline Entity CreateBezierSurface(Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		auto resultBezierSurface = scene->CreateEntity();

		AddShapeToEntity(resultBezierSurface, "Bezier Surface");
		AddBezierSurfaceToEntity(resultBezierSurface, scene, bezierParams);

		return resultBezierSurface;
	}
}
