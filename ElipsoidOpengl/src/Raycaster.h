#pragma once
#include "Shader.h"
#include "Vector4.h"
#include "Window.h"
class Raycaster
{
private:
	struct RaycasterVertexData
	{
		Algebra::Vector4 position;
		Algebra::Vector4 color;
	};

	unsigned int VBO, VAO;
	Shader usedShader;

	std::vector<RaycasterVertexData> textureData;

	void RunRays(Window* window, int xSize = 100, int ySize = 100);

	void SaveToBuffers();
public:
	Raycaster();
	void RenderResult(Window* window);
};

