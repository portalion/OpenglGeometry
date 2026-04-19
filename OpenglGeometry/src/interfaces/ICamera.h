#pragma once
#include <scene/Components.h>

class ICamera
{
public:
	virtual void HandleInput(CameraComponent& cameraComponent) = 0;
};