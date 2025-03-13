#pragma once
#include <vector>
#include <utils/GlCall.h>

class VertexBuffer
{
public:
	VertexBuffer();
	template<class T> VertexBuffer(std::vector<T> vertices);
	void Bind() const;
	void UnBind() const;

	template<class T> void AssignVectorData(std::vector<T> vertices) const;
private:
	unsigned int id;
};

template<class T>
inline VertexBuffer::VertexBuffer(std::vector<T> vertices)
	: VertexBuffer{}
{
	Bind();
	GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_DYNAMIC_DRAW));
	UnBind();
}

template<class T>
inline void VertexBuffer::AssignVectorData(std::vector<T> vertices) const
{
	Bind();
	GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_DYNAMIC_DRAW));
	UnBind();
}
