#pragma once
#include "VertexBuffer.h"
#include "VertexLayout.h"
class VertexArray
{
public:
	VertexArray(VertexBuffer vbo, VertexDataType dataType);

	void Bind() const;
	void UnBind() const;
private:
	unsigned int id;
	VertexBuffer vbo;
};

