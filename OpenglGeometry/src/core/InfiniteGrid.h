#pragma once
#include "Algebra.h"
#include <managers/ShaderManager.h>
#include <engine/Renderer.h>

class InfiniteGrid
{
private:
	Renderer<PositionVertexData> renderer;
	std::shared_ptr<Shader> shader;
public:
	InfiniteGrid();
	void Render(Algebra::Matrix4 viewMatrix, Algebra::Matrix4 projectionMatrix, Algebra::Vector4 cameraPosition);
};

