#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
	:id{ 0 }
{
	GLCall(glGenBuffers(1, &id));
}

IndexBuffer::IndexBuffer(std::vector<unsigned int> indices)
	:IndexBuffer{}
{
	AssignIndices(indices);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &id);
}

void IndexBuffer::AssignIndices(std::vector<unsigned int> indices) const
{
	Bind();
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW));
	UnBind();
}

void IndexBuffer::Bind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
}

void IndexBuffer::UnBind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
