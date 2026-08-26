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

	static const Algebra::Vector4 startingCameraTarget;
	static const float startingCameraDistance;
	static const float startingCameraPitch;

	static const Algebra::Vector4 defaultPointsColor;
	static const Algebra::Vector4 selectionColor;
};

