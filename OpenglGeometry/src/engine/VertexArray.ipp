#pragma once
#include "VertexArray.h"
#include "utils/GlCall.h"

template<typename VertexData>
inline VertexArray<VertexData>::VertexArray(const VertexBuffer& vbo)
	: id{ 0 }
{
	GLCall(glGenVertexArrays(1, &id));
	Bind();
	vbo.Bind();
	VertexAttribLayout<VertexData>::Enable();
	UnBind();
	vbo.UnBind();
}

template<typename VertexData>
inline VertexArray<VertexData>::VertexArray(const VertexBuffer& vbo, const IndexBuffer& ibo)
	: id{ 0 }
{
	GLCall(glGenVertexArrays(1, &id));
	Bind();
	vbo.Bind();
	ibo.Bind();
	VertexAttribLayout<VertexData>::Enable();
	UnBind();
	vbo.UnBind();
	ibo.UnBind();
}

template<typename VertexData>
inline VertexArray<VertexData>::~VertexArray()
{
	glDeleteVertexArrays(1, &id);
}

template<typename VertexData>
inline void VertexArray<VertexData>::Bind() const
{
	GLCall(glBindVertexArray(id));
}

template<typename VertexData>
inline void VertexArray<VertexData>::UnBind() const
{
	GLCall(glBindVertexArray(0));
}