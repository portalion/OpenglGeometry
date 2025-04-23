#pragma once
#include "ITransformationComponents.h"
#include "engine/Notifier.h"

class ObservablePositionComponent : public IPositionComponent
{
private:
	Algebra::Vector4 position;
	Notifier& notifier;
public:
	inline ObservablePositionComponent(Notifier& notifier) :notifier{ notifier } {}

	Algebra::Matrix4 GetModelMatrix() const override;
	Algebra::Vector4 GetPosition() const override;
	void SetPosition(Algebra::Vector4 position) override;
	void Move(Algebra::Vector4 translation) override;
};

class ObservableRotationComponent : public IRotationComponent
{
private:
	Algebra::Quaternion rotation;
	Notifier& notifier;
public:
	inline ObservableRotationComponent(Notifier& notifier) :notifier{ notifier } {}

	Algebra::Matrix4 GetModelMatrix() const override;
	Algebra::Quaternion GetRotation() const override;
	void SetRotation(Algebra::Quaternion rotation) override;
	void Rotate(Algebra::Quaternion rotation) override;
};

class ObservableScaleComponent : public IScaleComponent
{
private:
	float scale = 1.f;
	Notifier& notifier;
public:
	inline ObservableScaleComponent(Notifier& notifier) :notifier{ notifier } {}

	Algebra::Matrix4 GetModelMatrix() const override;
	float GetScale() const override;
	void SetScale(float scale) override;
	void Scale(float scale) override;
};

