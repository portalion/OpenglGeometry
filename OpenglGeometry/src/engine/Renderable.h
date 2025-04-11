#pragma once
#include <engine/Renderer.h>

template<typename T>
struct RenderableMesh
{
	std::vector<T> vertices;
	std::vector<unsigned int> indices;
};

template<typename T>
class Renderable
{
protected:
	Renderer<T> renderer;
	RenderingMode renderingMode = RenderingMode::TRIANGLES;

	void SaveMesh();

	virtual RenderableMesh<T> GenerateMesh() = 0;
public:
	inline Renderable(VertexDataType dataType);
	inline void Render() const;
};

#include "Renderable.ipp"