#pragma once
#include "Algebra.h"

class IPositionComponent {
public:
	virtual ~IPositionComponent() = default;

	virtual Algebra::Matrix4 GetModelMatrix() const = 0;
	virtual Algebra::Vector4 GetPosition() const = 0;
	virtual void SetPosition(Algebra::Vector4 pos) = 0;
	virtual void Move(Algebra::Vector4 translation) = 0;
};

class IRotationComponent {
public:
	virtual ~IRotationComponent() = default;

	virtual Algebra::Matrix4 GetModelMatrix() const = 0;
	virtual Algebra::Quaternion GetRotation() const = 0;
	virtual void SetRotation(Algebra::Quaternion rot) = 0;
	virtual void Rotate(Algebra::Quaternion rotation) = 0;
};

class IScaleComponent {
public:
	virtual ~IScaleComponent() = default;

	virtual Algebra::Matrix4 GetModelMatrix() const = 0;
	virtual float GetScale() const = 0;
	virtual void SetScale(float scale) = 0;
	virtual void Scale(float scale) = 0;
};
