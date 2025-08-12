#pragma once
#include "core/Base.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <vector>

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void Bind() const;
	void Unbind() const;

	void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
	void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

	const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
	const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	template <typename T>
	static Ref<VertexArray> CreateWithBuffers(std::vector<T>& vertices, std::vector<uint32_t>& indices, const BufferLayout& layout)
	{
		auto result = CreateRef<VertexArray>();
		auto VBO = CreateRef<VertexBuffer>();
		VBO->SetData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(T)));
		VBO->SetLayout(layout);
		result->AddVertexBuffer(VBO);
		auto IBO = CreateRef<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
		result->SetIndexBuffer(IBO);
		
		return result;
	}

private:
	uint32_t m_RendererID;
	uint32_t m_VertexBufferIndex = 0;
	std::vector<Ref<VertexBuffer>> m_VertexBuffers;
	Ref<IndexBuffer> m_IndexBuffer;
};
