#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
	struct BezierSurfaceCreationParameters
	{
		float sizeX = 4;
		float sizeY = 4;
		unsigned int numberOfXPatches = 1;
		unsigned int numberOfYPatches = 1;
	};

	inline std::vector<std::vector<Entity>> GenerateRectangularGridOfPoints(Scene* scene, BezierSurfaceCreationParameters params)
	{
		const unsigned int CONTROL_PONTS_PER_EDGE = 4;
		const unsigned int numberOfPointsX =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfXPatches + 1;
		const unsigned int numberOfPointsY =
			(CONTROL_PONTS_PER_EDGE - 1) * params.numberOfYPatches + 1;

		std::vector<std::vector<Entity>> result(numberOfPointsX, std::vector<Entity>(numberOfPointsY));

		const float sizeXPerPoint = params.sizeX / (numberOfPointsX - 1);
		const float sizeYPerPoint = params.sizeY / (numberOfPointsY - 1);

		for (int i = 0; i < numberOfPointsX; ++i)
		{
			for (int j = 0; j < numberOfPointsY; ++j)
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

	inline void FillRectangularBezierComponent(Scene* scene, BezierSurfaceGenerationComponent& result, BezierSurfaceCreationParameters bezierParams)
	{
		GenerateRectangularGridOfPoints(scene, bezierParams);
	}

	inline Entity AddBezierSurfaceToEntity(Entity entity, Scene* scene, BezierSurfaceCreationParameters bezierParams)
	{
		entity.AddTag<IsDirtyTag>();
		auto& bezierComponent = entity.AddComponent<BezierSurfaceGenerationComponent>();
		FillRectangularBezierComponent(scene, bezierComponent, bezierParams);
		

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
