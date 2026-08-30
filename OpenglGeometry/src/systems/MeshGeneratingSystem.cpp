#include "MeshGeneratingSystem.h"
#include "meshGenerators/MeshGenerators.h"
#include "scene/ObjectType.h"

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
	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, TorusGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		const auto& tgc = entity.GetComponent<TorusGenerationComponent>();

		auto generatedMesh = MeshGenerator::Torus::GenerateMesh(
			tgc.radius, tgc.tubeRadius, tgc.radialSegments, tgc.tubularSegments);

		ModifyOrCreateMesh(entity, generatedMesh.vertices, generatedMesh.indices, 
			generatedMesh.layout);
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
	BufferLayout bezierShaderLayout
	({
		{ ShaderDataType::Float4, "a_Position" }
	});

	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, BezierSurfaceGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();
		std::vector<Algebra::Vector4> vertices;
		auto patches = entity.GetComponent<BezierSurfaceGenerationComponent>().bezierPatches;

		const bool isC2 = GetObjectType(entity) == ObjectType::BezierSurfaceC2;

		uint32_t indice = 0;
		std::vector<uint32_t> indices;
		for(auto patchRow : patches)
			for(auto patch : patchRow)
			{
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

				for(int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++)
					{
						vertices.push_back(controlPositions[i][j]);
						indices.push_back(indice++);
					}
			}

		ModifyOrCreateMesh(entity, vertices, indices,
			bezierShaderLayout, RenderingMode::Patches, 
			{ AvailableShaders::BezierSurfaceHorizontal, AvailableShaders::BezierSurfaceVertical });
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
	SurfaceControlNetGeneration();
	LineGeneration();
	TorusGeneration();
}
