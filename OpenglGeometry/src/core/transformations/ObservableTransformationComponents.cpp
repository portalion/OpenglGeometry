#include "ObservableTransformationComponents.h"

Algebra::Matrix4 ObservablePositionComponent::GetModelMatrix() const
{
	return Algebra::Matrix4::Translation(position);
}

Algebra::Vector4 ObservablePositionComponent::GetPosition() const
{
    return position;
}

void ObservablePositionComponent::SetPosition(Algebra::Vector4 position)
{
	if (this->position != position)
	{
		this->position = position;
		notifier.Notify();
	}
}

void ObservablePositionComponent::Move(Algebra::Vector4 translation)
{
	this->position += translation;
	notifier.Notify();
}

Algebra::Matrix4 ObservableRotationComponent::GetModelMatrix() const
{
	return rotation.ToMatrix();
}

Algebra::Quaternion ObservableRotationComponent::GetRotation() const
{
    return rotation;
}

void ObservableRotationComponent::SetRotation(Algebra::Quaternion rot)
{
	if (this->rotation != rot)
	{
		this->rotation = rot;
		notifier.Notify();
	}
}

void ObservableRotationComponent::Rotate(Algebra::Quaternion rotation)
{
	this->rotation = this->rotation * rotation;
	notifier.Notify();
}

Algebra::Matrix4 ObservableScaleComponent::GetModelMatrix() const
{
	return Algebra::Matrix4::DiagonalScaling(scale, scale, scale);
}

float ObservableScaleComponent::GetScale() const
{
    return scale;
}

void ObservableScaleComponent::SetScale(float scale)
{
	if (this->scale != scale)
	{
		this->scale = scale;
		notifier.Notify();
	}
}

void ObservableScaleComponent::Scale(float scale)
{
	if (this->scale != 1.f)
	{
		this->scale *= scale;
		notifier.Notify();
	}
}
