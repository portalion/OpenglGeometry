#pragma once
#include <cmath>
#include "Algebra.h"

namespace Stereo
{
	struct EyeView
	{
		Algebra::Matrix4 view;
		Algebra::Matrix4 projection;
	};

	struct EyePair
	{
		EyeView left;
		EyeView right;
	};

	inline EyePair Compute(const Algebra::Matrix4& baseView, float aspect,
		float nearPlane, float farPlane, float fov, float eyeDistance, float convergence)
	{
		const float halfHeight = nearPlane * std::tan(fov / 2.f);
		const float halfWidth = halfHeight * aspect;
		const float halfEye = eyeDistance / 2.f;
		const float shift = convergence > 0.f ? halfEye * nearPlane / convergence : 0.f;

		EyePair result;

		result.left.view = Algebra::Matrix4::Translation(halfEye, 0.f, 0.f) * baseView;
		result.left.projection = Algebra::Matrix4::ProjectionOffAxis(
			-halfWidth + shift, halfWidth + shift, -halfHeight, halfHeight, nearPlane, farPlane);

		result.right.view = Algebra::Matrix4::Translation(-halfEye, 0.f, 0.f) * baseView;
		result.right.projection = Algebra::Matrix4::ProjectionOffAxis(
			-halfWidth - shift, halfWidth - shift, -halfHeight, halfHeight, nearPlane, farPlane);

		return result;
	}
}
