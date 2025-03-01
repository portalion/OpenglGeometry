#pragma once
#include "Shader.h"
#include "Vector4.h"
class Raycaster
{
private:
	struct RaycasterVertexData
	{
		Algebra::Vector4 positions;
		Algebra::Vector4 colors;
	};

	unsigned int VBO, VAO;
	Shader usedShader;

	void SaveToBuffers();
public:
	Raycaster();
	void RenderResult();
};

