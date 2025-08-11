#include "MeshGeneratingSystem.h"
#include "scene/Entity.h"

Algebra::Vector4 GetPoint(float angleTube, float angleRadius, float radius, float tubeRadius)
{
	return Algebra::Matrix4::RotationY(angleRadius) *
		Algebra::Vector4(radius + tubeRadius * cosf(angleTube), tubeRadius * sinf(angleTube), 0.f, 1.f);
}

void MeshGeneratingSystem::TorusGeneration()
{
	for (auto entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, TorusGenerationComponent>())
	{
		Entity e{ entity, m_Scene.get() };
		e.RemoveTag<IsDirtyTag>();

		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "position" }
		};

		const auto& tgc = e.GetComponent<TorusGenerationComponent>();

		std::vector<float> vertices;

		for (unsigned int i = 0; i < tgc.radialSegments; i++)
		{
			for (unsigned int j = 0; j < tgc.tubularSegments; j++)
			{
				const auto& point = GetPoint(2 * 3.14f * j / tgc.tubularSegments, 2 * 3.14f * i / tgc.radialSegments, tgc.radius, tgc.tubeRadius);
				vertices.push_back(point.x);
				vertices.push_back(point.y);
				vertices.push_back(point.z);
				vertices.push_back(point.w);
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

		if (!e.HasComponent<MeshComponent>())
		{
			auto meshComponent = &e.AddComponent<MeshComponent>();

			auto indexBuffer = CreateRef<IndexBuffer>(indices.data(), indices.size());
			auto vertexBuffer = CreateRef<VertexBuffer>(vertices.data(), vertices.size());
			vertexBuffer->SetLayout(layout);
			auto vertexArray = CreateRef<VertexArray>();
			vertexArray->AddVertexBuffer(vertexBuffer);
			vertexArray->SetIndexBuffer(indexBuffer);

			meshComponent->mesh = vertexArray;
			meshComponent->renderingMode = RenderingMode::Lines;
		}
		else
		{
			auto& mc = e.GetComponent<MeshComponent>();
			mc.renderingMode = RenderingMode::Lines;
			auto meshVAO = mc.mesh;
			meshVAO->GetVertexBuffers()[0]->SetData(vertices.data(), vertices.size());
			meshVAO->GetVertexBuffers()[0]->SetLayout(layout);
			meshVAO->GetIndexBuffer()->SetIndices(indices.data(), indices.size());
		}
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
