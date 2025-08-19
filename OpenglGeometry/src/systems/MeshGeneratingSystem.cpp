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

MeshGeneratingSystem::MeshGeneratingSystem(Ref<Scene> m_Scene)
	:m_Scene {m_Scene}
{
}

void MeshGeneratingSystem::Process()
{
	TorusGeneration();
}
