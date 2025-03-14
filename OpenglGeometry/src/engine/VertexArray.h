#pragma once
#include "VertexBuffer.h"
#include "VertexLayout.h"
#include "IndexBuffer.h"
class VertexArray
{
public:
	VertexArray(const VertexBuffer& vbo, VertexDataType dataType);
	VertexArray(const VertexBuffer& vbo, VertexDataType dataType, const IndexBuffer& ibo);
	~VertexArray();

	void Bind() const;
	void UnBind() const;
private:
	unsigned int id;
};

