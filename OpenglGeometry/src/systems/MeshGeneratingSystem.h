#pragma once
#include <scene/Scene.h>
#include "scene/Entity.h"
#include "scene/Components.h"
#include "ISystem.h"

class MeshGeneratingSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;

	std::vector<Algebra::Vector4> CopyValidPointsToVector(std::list<Entity>& pointEntities);
	std::vector<uint32_t> GenerateLineIndices(unsigned int verticesSize);

	void TorusGeneration();
	void LineGeneration();
	void BezierGeneration();

	template<typename T>
	void ModifyOrCreateMesh(Entity e, std::vector<T>& vertices, std::vector<uint32_t>& indices, const BufferLayout& layout, const RenderingMode& mode = RenderingMode::Lines, const AvailableShaders shaderType = AvailableShaders::Default);
public:
	MeshGeneratingSystem(Ref<Scene> m_Scene);

	void Process();
};

template<typename T>
inline void MeshGeneratingSystem::ModifyOrCreateMesh(Entity e, std::vector<T>& vertices, std::vector<uint32_t>& indices, const BufferLayout& layout, const RenderingMode& mode, const AvailableShaders shaderType)
{
	if (e.HasComponent<MeshComponent>())
	{
		auto& meshComponent = e.GetComponent<MeshComponent>();
		meshComponent.renderingMode = mode;
		meshComponent.shaderType = shaderType;

		auto vertexArray = meshComponent.mesh;
		vertexArray->GetVertexBuffers()[0]->SetData(vertices.data(),
			static_cast<uint32_t>(vertices.size() * sizeof(Algebra::Vector4)));
		vertexArray->GetVertexBuffers()[0]->SetLayout(layout);
		vertexArray->GetIndexBuffer()->SetIndices(indices.data(), 
			static_cast<uint32_t>(indices.size()));
	}
	else
	{
		auto& meshComponent = e.AddComponent<MeshComponent>();
		auto vertexArray = VertexArray::CreateWithBuffers(vertices, indices, layout);

		meshComponent.mesh = vertexArray;
		meshComponent.renderingMode = mode;
		meshComponent.shaderType = shaderType;
	}
}
