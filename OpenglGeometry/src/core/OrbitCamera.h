#pragma once
#include "Algebra.h"
#include "interfaces/ICamera.h"

class OrbitCamera : public ICamera
{
private:
	Algebra::Vector4 target;
	float distance;
	float yaw;
	float pitch;
	float roll;

	void HandlePan(const ViewportData& viewport);
	void HandleZoom();
	void HandleRotations(const ViewportData& viewport);
public:
	OrbitCamera(Algebra::Vector4 target = Algebra::Vector4(0.f, 0.f, 0.f, 1.f),
		float distance = 10.f, float pitch = 0.f, float yaw = 0.f);

	Algebra::Matrix4 GetRotationMatrix() const;
	Algebra::Matrix4 GetViewMatrix() const;

	inline Algebra::Vector4 GetTarget() const override { return target; }
	inline float GetDistance() const { return distance; }

	Algebra::Vector4 GetPosition() const override;
	Algebra::Vector4 GetRight() const override;
	Algebra::Vector4 GetUp() const override;
	Algebra::Vector4 GetForward() const override;
	void Focus(const Algebra::Vector4& point, float radius) override;

	void HandleInput(CameraComponent& cameraComponent, const ViewportData& viewport) override;
};
