#pragma once
#include "Renderer.h"

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
	RenderingModeLegacy renderingMode = RenderingModeLegacy::TRIANGLES;

	void SaveMesh();

	virtual RenderableMesh<T> GenerateMesh() = 0;
public:
	virtual ~Renderable() = default;
	inline Renderable();
	inline virtual void Render() const;
};

#include "Renderable.ipp"