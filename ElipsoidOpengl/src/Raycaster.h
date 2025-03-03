#pragma once
#include "Shader.h"
#include "Vector4.h"
#include "Window.h"
#include "RaycastableEllipsoid.h"

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

	RaycastableEllipsoid shape;

	int intensityPower;
	int rayThickness;
	bool shouldRecalculate;

	void RunRays(Window* window);
	float GetLightIntensity(Algebra::Vector4 observatorPosition, 
		Algebra::Vector4 point, Algebra::Vector4 gradient);
	void SaveToBuffers();
public:
	Raycaster();

	void RayCast(Window* window);
	void RenderMenu();
	void RenderResult();
};

