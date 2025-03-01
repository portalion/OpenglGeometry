#pragma once
#include "Matrix4.h"

class RaycastableEllipsoid
{
private:
	float a = 0.1f;
	float b = 0.1f;
	float c = 0.1f;

	Algebra::Matrix4 GetMatrix();
public:
	std::pair<bool, float> FindZ(float x, float y);
};

