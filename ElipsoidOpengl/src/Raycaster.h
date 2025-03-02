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

	void RunRays(Window* window, int thickness);
	float GetLightIntensity(Algebra::Vector4 observatorPosition, Algebra::Vector4 point, Algebra::Vector4 gradient, int m);
	void SaveToBuffers();
public:
	Raycaster();
	void RenderResult(Window* window);
};

