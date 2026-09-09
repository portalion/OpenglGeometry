#pragma once
#include <vector>
#include "Algebra.h"
#include "core/Base.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/ObjectType.h"
#include "meshGenerators/BezierSurfaceGenerator.h"
#include "geometry/ParametricSurface.h"

namespace Geometry
{
	inline Algebra::Matrix4 EntityModelMatrix(Entity entity)
	{
		Algebra::Matrix4 model = Algebra::Matrix4::Identity();
		if (entity.HasComponent<PositionComponent>())
		{
			model = model * Algebra::Matrix4::Translation(entity.GetComponent<PositionComponent>().position);
		}
		if (entity.HasComponent<RotationComponent>())
		{
			model = model * entity.GetComponent<RotationComponent>().rotation.ToMatrix();
		}
		if (entity.HasComponent<ScaleComponent>())
		{
			const Algebra::Vector4 s = entity.GetComponent<ScaleComponent>().scale;
			model = model * Algebra::Matrix4::DiagonalScaling(s.x, s.y, s.z);
		}
		return model;
	}

	inline bool IsParametricSurface(Entity entity)
	{
		if (!entity.IsValid())
		{
			return false;
		}
		const std::optional<ObjectType> type = GetObjectType(entity);
		return type && (*type == ObjectType::Torus
			|| *type == ObjectType::BezierSurfaceC0
			|| *type == ObjectType::BezierSurfaceC2);
	}

	inline Ref<IParametricSurface> MakeParametricSurface(Entity entity)
	{
		const std::optional<ObjectType> type = GetObjectType(entity);
		if (!type)
		{
			return nullptr;
		}

		if (*type == ObjectType::Torus)
		{
			const auto& torus = entity.GetComponent<TorusGenerationComponent>();
			return CreateRef<TorusParametricSurface>(torus.radius, torus.tubeRadius, EntityModelMatrix(entity));
		}

		if (*type != ObjectType::BezierSurfaceC0 && *type != ObjectType::BezierSurfaceC2)
		{
			return nullptr;
		}

		if (!entity.HasComponent<BezierSurfaceGenerationComponent>())
		{
			return nullptr;
		}

		const auto& surface = entity.GetComponent<BezierSurfaceGenerationComponent>();
		const bool isC2 = *type == ObjectType::BezierSurfaceC2;

		std::vector<std::vector<BezierSurfaceParametricSurface::Patch>> patches;
		for (const auto& patchRow : surface.bezierPatches)
		{
			std::vector<BezierSurfaceParametricSurface::Patch> row;
			for (Entity patchEntity : patchRow)
			{
				if (!patchEntity.IsValid() || !patchEntity.HasComponent<BezierPatchGenerationComponent>())
				{
					return nullptr;
				}

				const auto& control = patchEntity.GetComponent<BezierPatchGenerationComponent>().controlPoints;
				MeshGenerator::BezierSurfaceC2::PatchGrid grid{};
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						Entity point = control[i][j];
						if (!point.IsValid() || !point.HasComponent<PositionComponent>())
						{
							return nullptr;
						}
						Algebra::Vector4 position = point.GetComponent<PositionComponent>().position;
						position.w = 1.f;
						grid[i][j] = position;
					}
				}

				if (isC2)
				{
					grid = MeshGenerator::BezierSurfaceC2::DeBoorToBernstein(grid);
				}

				BezierSurfaceParametricSurface::Patch patch{};
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						patch[i][j] = grid[i][j];
					}
				}
				row.push_back(patch);
			}
			patches.push_back(std::move(row));
		}

		if (patches.empty() || patches[0].empty())
		{
			return nullptr;
		}

		return CreateRef<BezierSurfaceParametricSurface>(std::move(patches));
	}
}
