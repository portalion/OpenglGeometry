#pragma once
#include "Matrix4.h"

class RaycastableEllipsoid
{
private:
	float a = 1.f;
	float b = 1.f;
	float c = 1.f;

	Algebra::Matrix4 ReverseTransformations;
	Algebra::Matrix4 GetMatrix();
public:
	RaycastableEllipsoid();
	std::pair<bool, float> FindZ(float x, float y);
	Algebra::Vector4 FindGradient(float x, float y, float z);
	bool RenderMenu();
	
	void Translate(float x, float y, float z);
};

