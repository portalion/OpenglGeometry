#pragma once
#include "ITransformationComponents.h"

class EmptyPositionComponent final : public IPositionComponent {
public:
	inline Algebra::Matrix4 GetModelMatrix() const override { return Algebra::Matrix4::Identity(); }

	inline Algebra::Vector4 GetPosition() const override { return Algebra::Vector4(); }
	inline void SetPosition(Algebra::Vector4) override {}
	inline void Move(Algebra::Vector4) override {}

	static EmptyPositionComponent* GetInstance() { static EmptyPositionComponent instance; return &instance; }
};

class EmptyRotationComponent final : public IRotationComponent {
public:
	inline Algebra::Matrix4 GetModelMatrix() const override { return Algebra::Matrix4::Identity(); }

	inline Algebra::Quaternion GetRotation() const override { return Algebra::Quaternion::Identity(); }
	inline void SetRotation(Algebra::Quaternion) override {}
	inline void Rotate(Algebra::Quaternion) override {}

	static EmptyRotationComponent* GetInstance() { static EmptyRotationComponent instance; return &instance; }
};

class EmptyScaleComponent final : public IScaleComponent {
public:
	inline Algebra::Matrix4 GetModelMatrix() const override { return Algebra::Matrix4::Identity(); }

	inline float GetScale() const override { return 1.f; }
	inline void SetScale(float) override {}
	inline void Scale(float) override {}

	static EmptyScaleComponent* GetInstance() { static EmptyScaleComponent instance; return &instance; }
};
