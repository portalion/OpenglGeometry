#pragma once
#include "VertexArray.h"

template <class T> class Renderer
{
private:
	VertexBuffer vbo;
	IndexBuffer ibo;
	VertexArray vao;
public:
	Renderer(VertexDataType dataType);
	Renderer(VertexDataType dataType, std::vector<unsigned int> indices);
	Renderer(VertexDataType dataType, std::vector<T> vertices);
	Renderer(VertexDataType dataType, std::vector<T> vertices, std::vector<unsigned int> indices);

	void AssignIndices(std::vector<unsigned int> indices);
	void AssignVertices(std::vector<T> vertices);

	void Render(GLenum mode = GL_TRIANGLES);
};

#include "Renderer.ipp"