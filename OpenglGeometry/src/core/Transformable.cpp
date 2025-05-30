#include "Transformable.h"

void ScaleComponent::Scale(Algebra::Vector4  scale, bool isTorus)
{
	if (isTorus)
	{
		this->scale.x *= scale.x;
		this->scale.y *= scale.y;
		this->scale.z *= scale.z;
	}
}

void PositionComponent::Move(Algebra::Vector4 translation)
{
	position += translation;
}

void RotationComponent::Rotate(Algebra::Quaternion rotation, bool isTorus)
{
	if (isTorus)
	this->rotation = this->rotation * rotation;
	this->rotation.Normalize();
}