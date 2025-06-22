#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexAttribLayout.h"

template<typename VertexData>
class VertexArray
{
public:
	inline VertexArray(const VertexBuffer& vbo);
	inline VertexArray(const VertexBuffer& vbo, const IndexBuffer& ibo);
	~VertexArray();

	inline void Bind() const;
	inline void UnBind() const;
private:
	unsigned int id;
};

#include "VertexArray.ipp"