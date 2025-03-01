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
	
	float deltaSquared = sqrtf(delta);
	float z1 = (-b + deltaSquared) / (2.f * a);
	float z2 = (-b - deltaSquared) / (2.f * a);

	return {true, std::max(z1, z2)};
}
