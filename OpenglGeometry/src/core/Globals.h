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

	static const Algebra::Vector4 startingCameraPosition;

	static Algebra::Vector4 defaultPointsColor;
	static Algebra::Vector4 defaultMiddlePointColor;
	static Algebra::Vector4 defaultVirtualPointColor;
	static Algebra::Vector4 defaultLineColor;

	static void ChangeStereo(bool isStereo)
	{
		if (isStereo)
		{
			Globals::defaultPointsColor = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);
			Globals::defaultMiddlePointColor = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);
			Globals::defaultVirtualPointColor = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);
			Globals::defaultLineColor = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);
		}
		else
		{
			Globals::defaultPointsColor = Algebra::Vector4(1.f, 0.2f, 0.f, 1.f);
			Globals::defaultMiddlePointColor = Algebra::Vector4(1.f, 1.f, 1.f, 1.f);
			Globals::defaultVirtualPointColor = Algebra::Vector4(0.3f, 0.3f, 0.7f, 1.f);
			Globals::defaultLineColor = Algebra::Vector4(0.f, 0.f, 1.f, 1.f);
		}
	}
};

