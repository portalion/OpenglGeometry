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

void MeshGeneratingSystem::PolylineGeneration()
{
	for(Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, LineGenerationComponent>())
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

void MeshGeneratingSystem::BezierC0Generation()
{
	for(Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, BezierC0GenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();
		auto& bezierComponent = entity.GetComponent<BezierC0GenerationComponent>();
		auto& points = bezierComponent.controlPoints;

		std::vector<Algebra::Vector4> positions =
			CopyValidPointsToVector(bezierComponent.controlPoints);

		auto generatedMesh = MeshGenerator::BezierCurve::GenerateMesh(positions);

		ModifyOrCreateMesh(entity, generatedMesh.vertices, generatedMesh.indices,
			generatedMesh.layout, generatedMesh.renderingMode, generatedMesh.shaderType);
	}
}

MeshGeneratingSystem::MeshGeneratingSystem(Ref<Scene> m_Scene)
	:m_Scene {m_Scene}
{
}

void MeshGeneratingSystem::Process()
{
	BezierC0Generation();
	PolylineGeneration();
	TorusGeneration();
}
