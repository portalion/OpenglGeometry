#include "MeshGeneratingSystem.h"
#include "meshGenerators/MeshGenerators.h"

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
	for (int i = 0; i < static_cast<int>(verticesSize) - 1; i++)
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
			generatedMesh.layout, generatedMesh.renderingMode, generatedMesh.shaderType);
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
			generatedMesh.layout, generatedMesh.renderingMode, generatedMesh.shaderType);
	}
}

void MeshGeneratingSystem::BezierGeneration()
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
		auto indices = GenerateLineIndices(vertices.size());

		ModifyOrCreateMesh(entity, vertices, indices,
			bezierShaderLayout, RenderingMode::Patches, AvailableShaders::BezierCurveC0);
	}
}

MeshGeneratingSystem::MeshGeneratingSystem(Ref<Scene> m_Scene)
	:m_Scene {m_Scene}
{
}

void MeshGeneratingSystem::Process()
{
	BezierGeneration();
	LineGeneration();
	TorusGeneration();
}
