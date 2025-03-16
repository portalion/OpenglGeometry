#pragma once
#include "engine/Renderer.h"

class Torus
{
private:
	Renderer<PositionVertexData> renderer;

	float radius = 15.f;
	float tubeRadius = 5.f;
	unsigned int tubeSegments = 5;
	unsigned int radiusSegments = 3;

	Algebra::Vector4 GetPoint(float angleTube, float angleRadius);

	void GenerateAndSaveMesh();
public:
	Torus();
	void Render();
};

