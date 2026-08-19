#pragma once
#include <Vector4.h>

/// <summary>
/// struct for configuration constants, or global constants
/// </summary>
struct Globals
{
	//Scene / window / interface sizes
	static const int startingSceneWidth;
	static const int startingSceneHeight;
	static const int rightInterfaceWidth;

	//Camera frustum
	static const float cameraNearPlane;
	static const float cameraFarPlane;
	static const float cameraFieldOfView;

	static const Algebra::Vector4 startingCameraPosition;

	static const Algebra::Vector4 defaultPointsColor;
};

