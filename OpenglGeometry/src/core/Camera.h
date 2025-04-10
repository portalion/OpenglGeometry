#pragma once
#include "Algebra.h"

class Camera
{
private:
	float zoom;
	Algebra::Quaternion rotation;
	
	Algebra::Vector4 position;

	void HandleTranslation(const float& dt);
	void HandleZoom(const float& dt);
	void HandleRotations(const float& dt);
public:
	Camera(Algebra::Vector4 position = Algebra::Vector4(0.f, 0.f, 0.f, 1.f), float zoom = 1.f);
	
	Algebra::Matrix4 GetTranslationMatrix();
	Algebra::Matrix4 GetZoomMatrix();
	Algebra::Matrix4 GetRotationMatrix();

	inline Algebra::Vector4 GetPosition() { return position; }
	inline Algebra::Matrix4 GetViewMatrix() { return GetRotationMatrix() * GetTranslationMatrix() * GetZoomMatrix(); }

	void HandleInput();
};