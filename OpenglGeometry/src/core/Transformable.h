#pragma once
#include <Algebra.h>

class PositionComponent
{
private:
	Algebra::Vector4 position;
public:
	virtual ~PositionComponent() = default;
	inline virtual Algebra::Matrix4 GetModelMatrix() const { return Algebra::Matrix4::Translation(position); };
	inline Algebra::Vector4 GetPosition() const { return position; };
	inline void SetPosition(Algebra::Vector4 pos) { position = pos; }

	void Move(Algebra::Vector4 translation);
};

class RotationComponent
{
private:
	Algebra::Quaternion rotation;
public:
	virtual ~RotationComponent() = default;
	inline virtual Algebra::Matrix4 GetModelMatrix() const { return rotation.ToMatrix(); };
	inline Algebra::Quaternion GetRotation() const { return rotation; }
	inline void SetRotation(Algebra::Quaternion rot) { rotation = rot.Normalize(); }

	void Rotate(Algebra::Quaternion rotation, bool isTorus = false);
};

class ScaleComponent
{
private:
	float scale = 1.f;
public:
	virtual ~ScaleComponent() = default;
	inline virtual Algebra::Matrix4 GetModelMatrix() const { return Algebra::Matrix4::DiagonalScaling(scale, scale, scale); };
	inline float GetScale() const { return scale; }
	inline void SetScale(float scale) { this->scale = scale; }

	void Scale(float scale, bool isTorus = false);
};

class Transformable : public virtual PositionComponent, public virtual RotationComponent, public virtual ScaleComponent
{
public:
	inline Algebra::Matrix4 GetModelMatrix() const
	{
		return PositionComponent::GetModelMatrix() * RotationComponent::GetModelMatrix() * ScaleComponent::GetModelMatrix();
	}
};