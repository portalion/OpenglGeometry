#pragma once
#include "Algebra.h"

class Camera
{
private:
	float zoom;
	float yAngle;
	float xAngle;
	float zAngle;
	
	Algebra::Vector4 position;
	Algebra::Matrix4 GetTranslationMatrix();
	Algebra::Matrix4 GetZoomMatrix();
	Algebra::Matrix4 GetRotationMatrix();

	void HandleTranslation(const float& dt);
	void HandleZoom(const float& dt);
	void HandleRotations(const float& dt);
public:
	Camera(Algebra::Vector4 position = Algebra::Vector4(0.f, 0.f, 0.f, 1.f), float zoom = 1.f);
	
	inline Algebra::Vector4 GetPosition() { return position; }
	inline Algebra::Matrix4 GetViewMatrix() { return GetRotationMatrix() * GetTranslationMatrix() * GetZoomMatrix(); }

	void HandleInput();
};