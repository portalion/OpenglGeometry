#pragma once
#include <scene/Components.h>
#include <core/Viewport.h>

class ICamera
{
public:
	virtual ~ICamera() = default;

	virtual void HandleInput(CameraComponent& cameraComponent, const ViewportData& viewport) = 0;

	virtual Algebra::Vector4 GetPosition() const = 0;
	virtual Algebra::Vector4 GetTarget() const = 0;

	virtual Algebra::Vector4 GetRight() const = 0;
	virtual Algebra::Vector4 GetUp() const = 0;
	virtual Algebra::Vector4 GetForward() const = 0;

	virtual void Focus(const Algebra::Vector4& point, float radius) = 0;
};
