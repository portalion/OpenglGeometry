#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
	const unsigned int CONTROL_PONTS_PER_EDGE = 4;

	struct BezierSurfaceCreationParameters
	{
		float sizeX = 4;
		float sizeY = 4;
		unsigned int numberOfXPatches = 1;
		unsigned int numberOfYPatches = 1;
	};

	inline std::pair<unsigned int, unsigned int> 
		CalculateNumberOfPointsInRectangularGrid(BezierSurfaceCreationParameters params)
	{
		const unsigned int numberOfPointsX =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfXPatches + 1;
		const unsigned int numberOfPointsY =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfYPatches + 1;
		return { numberOfPointsX, numberOfPointsY };
	}

	inline std::vector<std::vector<Entity>> GenerateRectangularGridOfPoints(Scene* scene, BezierSurfaceCreationParameters params)
	{
		//TODO: move out to params
		auto [ numberOfPointsX, numberOfPointsY ] = CalculateNumberOfPointsInRectangularGrid(params);

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

	inline Entity CreateVirtualPatch(Scene* scene, Entity parentEntity)
	{
		auto entity = scene->CreateEntity();

		entity.AddComponent<BezierPatchGenerationComponent>();
		entity.AddComponent<VirtualEntityComponent>().realEntity = parentEntity;
			
		return entity;
	}

	inline void FillRectangularBezierComponent(Scene* scene, Entity surface,
		BezierSurfaceGenerationComponent& result, 
		BezierSurfaceCreationParameters bezierParams)
	{
		auto points = GenerateRectangularGridOfPoints(scene, bezierParams);
		result.bezierPatches = 
			std::vector<std::vector<Entity>>(bezierParams.numberOfXPatches,
				std::vector<Entity>(bezierParams.numberOfYPatches));

		for(int i = 0; i < bezierParams.numberOfXPatches; i++)
		{
			for(int j = 0; j < bezierParams.numberOfYPatches; j++)
			{
				auto patch = CreateVirtualPatch(scene, surface);
				result.bezierPatches[i][j] = patch;
				auto& patchPoints = patch.GetComponent<BezierPatchGenerationComponent>().controlPoints;

				for(int x = 0; x < CONTROL_PONTS_PER_EDGE; ++x)
				{
					for(int y = 0; y < CONTROL_PONTS_PER_EDGE; ++y)
					{
						patchPoints[x][y] = points[i * (CONTROL_PONTS_PER_EDGE - 1) + x]
												   [j * (CONTROL_PONTS_PER_EDGE - 1) + y];
					}
				}
			}
		}
	}

	inline Entity AddBezierSurfaceToEntity(Entity entity, Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		entity.AddTag<IsDirtyTag>();
		auto& bezierComponent = entity.AddComponent<BezierSurfaceGenerationComponent>();
		FillRectangularBezierComponent(scene, entity, bezierComponent, bezierParams);

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
