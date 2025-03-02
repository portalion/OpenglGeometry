#include "RaycastableEllipsoid.h"

Algebra::Matrix4 RaycastableEllipsoid::GetMatrix()
{
	return Algebra::Matrix4(a, b, c, -1.f);
}

std::pair<bool, float> RaycastableEllipsoid::FindZ(float x, float y)
{
	Algebra::Matrix4 ellipsoidMatrix = GetMatrix();
	float a = ellipsoidMatrix[2][2];
	float b = Algebra::Vector4(x, y, 0, 1) * ellipsoidMatrix * Algebra::Vector4(0, 0, 1, 0) + 
			  Algebra::Vector4(0, 0, 1, 0) * ellipsoidMatrix * Algebra::Vector4(x, y, 0, 1);
	float c = Algebra::Vector4(x, y, 0, 1) * ellipsoidMatrix * Algebra::Vector4(x, y, 0, 1);

	float delta = b * b - 4 * a * c;

	if (delta < 0)
	{
		return { false, 0.f };
	}

	if (abs(delta) < 1e-5)
	{
		float da = 10;
	}
	
	float deltaSquared = sqrtf(delta);
	float z1 = (-b + deltaSquared) / (2.f * a);
	float z2 = (-b - deltaSquared) / (2.f * a);

	return {true, std::max(z1, z2)};
}

Algebra::Vector4 RaycastableEllipsoid::FindGradient(float x, float y, float z)
{
	Algebra::Matrix4 ellipsoidMatrix = GetMatrix();
	float a = ellipsoidMatrix[2][2];
	float b = Algebra::Vector4(x, y, 0, 1) * ellipsoidMatrix * Algebra::Vector4(0, 0, 1, 0) +
		Algebra::Vector4(0, 0, 1, 0) * ellipsoidMatrix * Algebra::Vector4(x, y, 0, 1);
	
	float zGradient = 2 * a * z + b;

	a = ellipsoidMatrix[1][1];
	b = Algebra::Vector4(x, 0, z, 1) * ellipsoidMatrix * Algebra::Vector4(0, 1, 0, 0) +
		Algebra::Vector4(0, 1, 0, 0) * ellipsoidMatrix * Algebra::Vector4(x, 0, z, 1);
	
	float yGradient = 2 * a * y + b;

	a = ellipsoidMatrix[0][0];
	b = Algebra::Vector4(0, y, z, 1) * ellipsoidMatrix * Algebra::Vector4(1, 0, 0, 0) +
		Algebra::Vector4(1, 0, 0, 0) * ellipsoidMatrix * Algebra::Vector4(0, y, z, 1);

	float xGradient = 2 * a * x + b;


	return Algebra::Vector4(xGradient, yGradient, zGradient, 0.f);
}
