#pragma once
#include "VertexArray.h"
#include <memory>

enum class RenderingModeLegacy
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
public:
	Renderer();
	Renderer(std::vector<unsigned int> indices);
	Renderer(std::vector<T> vertices);
	Renderer(std::vector<T> vertices, std::vector<unsigned int> indices);

	void AssignIndices(std::vector<unsigned int> indices);
	void AssignVertices(std::vector<T> vertices);

	void Render(RenderingModeLegacy mode = RenderingModeLegacy::TRIANGLES) const;
};

#include "Renderer.ipp"