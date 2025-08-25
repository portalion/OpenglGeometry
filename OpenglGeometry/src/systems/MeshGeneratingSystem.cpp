#include "MeshGeneratingSystem.h"

Algebra::Vector4 GetPoint(float angleTube, float angleRadius, float radius, float tubeRadius)
{
	return Algebra::Matrix4::RotationY(angleRadius) *
		Algebra::Vector4(radius + tubeRadius * cosf(angleTube), tubeRadius * sinf(angleTube), 0.f, 1.f);
}

void MeshGeneratingSystem::TorusGeneration()
{
	for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, TorusGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();

		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};

		const auto& tgc = entity.GetComponent<TorusGenerationComponent>();

		std::vector<Algebra::Vector4> vertices;

		for (unsigned int i = 0; i < tgc.radialSegments; i++)
		{
			for (unsigned int j = 0; j < tgc.tubularSegments; j++)
			{
				const auto& point = GetPoint(2 * 3.14f * j / tgc.tubularSegments, 2 * 3.14f * i / tgc.radialSegments, tgc.radius, tgc.tubeRadius);
				vertices.push_back(point);
			}
		}

		std::vector<uint32_t> indices;

		for (unsigned int i = 0; i < tgc.radialSegments; i++)
		{
			int iNext = (i + 1) % tgc.radialSegments;
			for (unsigned int j = 0; j < tgc.tubularSegments; j++)
			{
				int jNext = (j + 1) % tgc.tubularSegments;
				indices.push_back(tgc.tubularSegments * i + j);
				indices.push_back(tgc.tubularSegments * i + jNext);
				indices.push_back(tgc.tubularSegments * i + j);
				indices.push_back((tgc.tubularSegments * iNext) + j);
			}
		}

		ModifyOrCreateMesh(entity, vertices, indices, layout);
	}
}

void MeshGeneratingSystem::PolylineGeneration()
{
	for(Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, LineGenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};

		auto& lineComponent = entity.GetComponent<LineGenerationComponent>();
		
		std::vector<Algebra::Vector4> vertices;
		std::vector<uint32_t> indices;

		for(auto it = lineComponent.controlPoints.begin(); it != lineComponent.controlPoints.end(); )
		{
			if (!it->IsValid() || !it->HasComponent<PositionComponent>())
			{
				it = lineComponent.controlPoints.erase(it);
				continue;
			}

			Algebra::Vector4 position = it->GetComponent<PositionComponent>().position;
			position.w = 1.f;
			vertices.push_back(position);
			indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
			indices.push_back(static_cast<uint32_t>(vertices.size()));
			it++;
		}

		if (!indices.empty())
		{
			indices.pop_back(); 
		}

		ModifyOrCreateMesh(entity, vertices, indices, layout);
	}
}

void MeshGeneratingSystem::BezierC0Generation()
{
	for(Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, BezierC0GenerationComponent>())
	{
		entity.RemoveTag<IsDirtyTag>();
		auto& bezierComponent = entity.GetComponent<BezierC0GenerationComponent>();
		auto& points = bezierComponent.controlPoints;

		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};

		MeshComponent mesh;

		std::vector<Algebra::Vector4> vertices;
		std::vector<uint32_t> indices;
		if (points.empty())
		{
			this->ModifyOrCreateMesh(entity, vertices, indices, layout, RenderingMode::Patches, AvailableShaders::BezierCurveC0);
			continue;
		}

		int i = 0;
		for (auto it = points.begin(); it != points.end();)
		{
			if (!it->IsValid() || !it->HasComponent<PositionComponent>())
			{
				it = points.erase(it);
				continue;
			}

			auto position = it->GetComponent<PositionComponent>().position.value;
			position.w = 1.f;
			vertices.push_back(position);
			indices.push_back(vertices.size() - 1);
			if (++i % 4 == 0)
			{
				indices.push_back(vertices.size() - 1);
				i++;
			}
			++it;
		}
		int fix = vertices.size() % 4;
		if (fix == 1) {
			// Duplicate the single point to create a degenerate cubic
			for (int i = 0; i < 4; i++)
			{
				indices.push_back(vertices.size() - 1);
			}
		}
		else if (fix == 2) {
			// Create a straight line cubic: start and end duplicated
			indices.push_back(vertices.size() - 2);
			indices.push_back(vertices.size() - 2);
			indices.push_back(vertices.size() - 1);
			indices.push_back(vertices.size() - 1);
		}
		else if (fix == 3) {
			// Upgrade quadratic to cubic
			auto p0 = vertices[vertices.size() - 3];
			auto p1 = vertices[vertices.size() - 2];
			auto p2 = vertices[vertices.size() - 1];
			vertices.pop_back();
			vertices.pop_back();
			vertices.pop_back();
			vertices.push_back(p0);
			vertices.push_back({(1.0f / 3.0f) * p0 + (2.0f / 3.0f) * p1});
			vertices.push_back({(2.0f / 3.0f) * p1 + (1.0f / 3.0f) * p2 });
			vertices.push_back(p2);
			indices.push_back(vertices.size() - 1);
		}

		ModifyOrCreateMesh(entity, vertices, indices, layout, RenderingMode::Patches, AvailableShaders::BezierCurveC0);
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
