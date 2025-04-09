#pragma once
#include <Algebra.h>

class PositionComponent
{
private:
	Algebra::Vector4 position;
public:
	inline Algebra::Matrix4 GetTranslationMatrix() const { return Algebra::Matrix4::Translation(position); };
	inline Algebra::Vector4 GetPosition() const { return position; };
	inline void SetPosition(Algebra::Vector4 pos) { position = pos; }

	void Move(Algebra::Vector4 translation);
};

class RotationComponent
{
private:
	Algebra::Quaternion rotation;
public:
	inline Algebra::Matrix4 GetRotationMatrix() const { return rotation.ToMatrix(); };
	inline Algebra::Quaternion GetRotation() const { return rotation; }
	inline void SetRotation(Algebra::Quaternion rot) { rotation = rot.Normalize(); }

	void Rotate(Algebra::Quaternion rotation);
};

class ScaleComponent
{
private:
	float scale = 1.f;
public:
	inline Algebra::Matrix4 GetScaleMatrix() const { return Algebra::Matrix4::DiagonalScaling(scale, scale, scale); };
	inline float GetScale() const { return scale; }
	inline void SetScale(float scale) { this->scale = scale; }

	void Scale(float scale);
};

class Transformable
{
private:
	PositionComponent positionComponent;
	RotationComponent rotationComponent;
	ScaleComponent scaleComponent;

public:
	inline void SetPosition(Algebra::Vector4 position) { positionComponent.SetPosition(position); }
	inline Algebra::Vector4 GetPosition() const { return positionComponent.GetPosition(); }
	inline void Move(Algebra::Vector4 translation) { positionComponent.Move(translation); }

	inline void SetRotation(Algebra::Quaternion rotation) { rotationComponent.SetRotation(rotation); }
	inline Algebra::Quaternion GetRotation() const { return rotationComponent.GetRotation(); }
	inline void Rotate(Algebra::Quaternion rotation) { rotationComponent.Rotate(rotation); }

	inline void SetScale(float scale) { scaleComponent.SetScale(scale); }
	inline float GetScale() const { return scaleComponent.GetScale(); }
	inline void Scale(float scale) { scaleComponent.Scale(scale); }

	inline Algebra::Matrix4 GetModelMatrix()
	{
		return positionComponent.GetTranslationMatrix() * rotationComponent.GetRotationMatrix() * scaleComponent.GetScaleMatrix();
	};
};