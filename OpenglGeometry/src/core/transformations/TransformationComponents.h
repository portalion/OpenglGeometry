#pragma once
#include "ITransformationComponents.h"

class PositionComponent : public IPositionComponent
{
private:
	Algebra::Vector4 position;
public:
	 ~PositionComponent() = default;
	inline Algebra::Matrix4 GetModelMatrix() const { return Algebra::Matrix4::Translation(position); };

	inline Algebra::Vector4 GetPosition() const { return position; };
	inline void SetPosition(Algebra::Vector4 pos) { position = pos; }
	inline void Move(Algebra::Vector4 translation) { position += translation; }
};

class RotationComponent : public IRotationComponent
{
private:
	Algebra::Quaternion rotation;
public:
	 ~RotationComponent() = default;
	inline Algebra::Matrix4 GetModelMatrix() const { return rotation.ToMatrix(); };

	inline Algebra::Quaternion GetRotation() const { return rotation; }
	inline void SetRotation(Algebra::Quaternion rot) { rotation = rot.Normalize(); }
	inline void Rotate(Algebra::Quaternion rotation) 
	{
		this->rotation = this->rotation * rotation;
		this->rotation.Normalize();
	}
};

class ScaleComponent : public IScaleComponent
{
private:
	float scale = 1.f;
public:
	~ScaleComponent() = default;
	inline  Algebra::Matrix4 GetModelMatrix() const { return Algebra::Matrix4::DiagonalScaling(scale, scale, scale); };

	inline float GetScale() const { return scale; }
	inline void SetScale(float scale) { this->scale = scale; }
	inline void Scale(float scale) { this->scale *= scale; }
};
