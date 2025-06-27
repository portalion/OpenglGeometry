#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexAttribLayout.h"

class VertexArray
{
public:
	VertexArray() = delete;
	~VertexArray();

	inline void Bind() const;
	inline void UnBind() const;

	template<typename VertexData>
	static inline VertexArray Create(const VertexBuffer& vbo);
	template<typename VertexData>
	static inline VertexArray Create(const VertexBuffer& vbo, const IndexBuffer& ibo);
private:
	unsigned int id;
};

#include "VertexArray.ipp"