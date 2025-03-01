#pragma once
#include "Shader.h"

class Raycaster
{
private:
	unsigned int VBO, VAO;
	Shader usedShader;

	void SaveToBuffers();
public:
	Raycaster();
	void RenderResult();
};

