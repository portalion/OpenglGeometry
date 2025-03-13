#include "VertexArray.h"
#include <utils/GlCall.h>

VertexArray::VertexArray(VertexBuffer vbo, const VertexLayout& layout)
: id{ 0 }, vbo{ vbo }
{
	GLCall(glGenVertexArrays(1, &id));
	Bind();
	vbo.Bind();
	layout.EnableAttribArrays();
	UnBind();
}


void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(id));
}

void VertexArray::UnBind() const
{
	GLCall(glBindVertexArray(0));
}
