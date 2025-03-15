#pragma once
#include <vector>
#include <utils/GlCall.h>
#include <iostream>

class VertexBuffer
{
private:
	size_t length;
public:
	VertexBuffer();
	~VertexBuffer();

	inline size_t GetLength() const { return length; };

	template<class T> VertexBuffer(std::vector<T> vertices);
	void Bind() const;
	void UnBind() const;

	template<class T> void AssignVectorData(std::vector<T> vertices);
private:
	unsigned int id;
};

template<class T>
inline VertexBuffer::VertexBuffer(std::vector<T> vertices)
	: VertexBuffer{}
{
	this->AssignVectorData(vertices);
}

template<class T>
inline void VertexBuffer::AssignVectorData(std::vector<T> vertices)
{
	length = vertices.size();
	Bind();
	GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_DYNAMIC_DRAW));
	UnBind();
}
