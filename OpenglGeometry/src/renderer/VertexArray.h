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

	void AddVertexBuffer(Unique<VertexBuffer> vertexBuffer);
	void SetIndexBuffer(Unique<IndexBuffer> indexBuffer);

	const std::vector<Unique<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
	const Unique<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	template <typename T>
	static Ref<VertexArray> CreateWithBuffers(std::vector<T>& vertices, std::vector<uint32_t>& indices, const BufferLayout& layout)
	{
		auto result = CreateRef<VertexArray>();
		auto VBO = CreateUnique<VertexBuffer>();
		VBO->SetData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(T)));
		VBO->SetLayout(layout);
		result->AddVertexBuffer(std::move(VBO));
		auto IBO = CreateUnique<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
		result->SetIndexBuffer(std::move(IBO));
		
		return result;
	}

private:
	uint32_t m_RendererID;
	uint32_t m_VertexBufferIndex = 0;
	std::vector<Unique<VertexBuffer>> m_VertexBuffers;
	Unique<IndexBuffer> m_IndexBuffer;
};
