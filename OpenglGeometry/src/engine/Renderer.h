#pragma once
#include "VertexArray.h"

enum class RenderingMode
{
	LINES = GL_LINES,
	TRIANGLES = GL_TRIANGLES,
	PATCHES = GL_PATCHES
};

template <class T> class Renderer
{
private:
	VertexBuffer vbo;
	IndexBuffer ibo;
	VertexArray<T> vao;
public:
	Renderer();
	Renderer(std::vector<unsigned int> indices);
	Renderer(std::vector<T> vertices);
	Renderer(std::vector<T> vertices, std::vector<unsigned int> indices);

	void AssignIndices(std::vector<unsigned int> indices);
	void AssignVertices(std::vector<T> vertices);

	void Render(RenderingMode mode = RenderingMode::TRIANGLES) const;
};

#include "Renderer.ipp"