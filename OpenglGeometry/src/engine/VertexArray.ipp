#pragma once
#include "VertexArray.h"
#include "utils/GlCall.h"

template<typename VertexData>
inline VertexArray VertexArray::Create(const VertexBuffer& vbo)
{
	VertexArray result;
	GLCall(glGenVertexArrays(1, &result.id));
	Bind();
	vbo.Bind();
	VertexAttribLayout<VertexData>::Enable();
	UnBind();
	vbo.UnBind();

	return result;
}

template<typename VertexData>
inline VertexArray VertexArray::Create(const VertexBuffer& vbo, const IndexBuffer& ibo)
{
	VertexArray result;
	GLCall(glGenVertexArrays(1, &result.id));
	Bind();
	vbo.Bind();
	ibo.Bind();
	VertexAttribLayout<VertexData>::Enable();
	UnBind();
	vbo.UnBind();
	ibo.UnBind();

	return result;
}

inline VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &id);
}

inline void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(id));
}

inline void VertexArray::UnBind() const
{
	GLCall(glBindVertexArray(0));
}