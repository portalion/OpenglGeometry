#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
	:length{ 0 }, id{ 0 }
{
	GLCall(glGenBuffers(1, &id));
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &id);
}

void VertexBuffer::Bind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, id));
}

void VertexBuffer::UnBind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
