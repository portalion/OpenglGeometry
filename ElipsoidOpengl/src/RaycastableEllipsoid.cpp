#include "RaycastableEllipsoid.h"
#include <imgui/imgui.h>

Algebra::Matrix4 RaycastableEllipsoid::GetMatrix()
{
	auto reverseScaleMatrix = Algebra::Matrix4::DiagonalScaling(1.f / scale, 1.f / scale, 1.f / scale);
	auto ellispoidParameters = Algebra::Matrix4(a, b, c, -1.f);

	auto transformationMatrix = reverseScaleMatrix * ReverseRotations * ReverseTranslations;

	return transformationMatrix.Transpose() * ellispoidParameters *
		transformationMatrix;
}

RaycastableEllipsoid::RaycastableEllipsoid()
{
	ReverseTranslations = Algebra::Matrix4::Identity();
	ReverseRotations = Algebra::Matrix4::Identity();
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

bool RaycastableEllipsoid::RenderMenu()
{
	bool somethingChanged = false;
	if (ImGui::CollapsingHeader("Ellipsoid", ImGuiTreeNodeFlags_Leaf))
	{
		somethingChanged |= ImGui::InputFloat("a", &a, 0.1f);
		somethingChanged |= ImGui::InputFloat("b", &b, 0.1f);
		somethingChanged |= ImGui::InputFloat("c", &c, 0.1f);
		somethingChanged |= ImGui::InputFloat("scale", &scale, 0.1f);
	}
	return somethingChanged;
}

void RaycastableEllipsoid::Translate(float x, float y, float z)
{
	ReverseTranslations = ReverseTranslations * Algebra::Matrix4::Translation(-x, -y, -z);
}

void RaycastableEllipsoid::AddRotation(Algebra::Matrix4 rot)
{
	ReverseRotations = ReverseRotations * rot;
}
