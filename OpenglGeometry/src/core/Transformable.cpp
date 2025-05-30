#include "Transformable.h"

void ScaleComponent::Scale(float scale, bool isTorus)
{
	if(isTorus)
	this->scale *= scale;
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