#pragma once
#include "VertexArray.h"
#include <memory>

enum class RenderingMode
{
	LINES = GL_LINES,
	TRIANGLES = GL_TRIANGLES,
	PATCHES = GL_PATCHES
};

class MeshApplier
{
private:
	VertexBuffer vbo;
	IndexBuffer ibo;
	std::unique_ptr<VertexArray> vao;
public:
	template<typename T>
	MeshApplier();
	template<typename T>
	MeshApplier(std::vector<unsigned int> indices);
	template<typename T>
	MeshApplier(std::vector<T> vertices);
	template<typename T>
	MeshApplier(std::vector<T> vertices, std::vector<unsigned int> indices);

	void AssignIndices(std::vector<unsigned int> indices);
	template<typename T>
	void AssignVertices(std::vector<T> vertices);

	void Render(RenderingMode mode = RenderingMode::TRIANGLES) const;
};

#include "MeshApplier.ipp"