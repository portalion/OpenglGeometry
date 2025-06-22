#pragma once
#include "Renderable.h"

template<typename T>
inline void Renderable<T>::SaveMesh()
{
	const RenderableMesh<T>& mesh = GenerateMesh();
	renderer.AssignVertices(mesh.vertices);
	renderer.AssignIndices(mesh.indices);
}

template<typename T>
inline Renderable<T>::Renderable()
	: renderer()
{
}

template<typename T>
inline void Renderable<T>::Render() const
{
	renderer.Render(renderingMode);
}