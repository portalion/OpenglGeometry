#include "Transformable.h"

void ScaleComponent::Scale(float scale)
{
//	this->scale *= scale;
}

void PositionComponent::Move(Algebra::Vector4 translation)
{
	position += translation;
}

void RotationComponent::Rotate(Algebra::Quaternion rotation)
{
//	this->rotation = this->rotation * rotation;
	this->rotation.Normalize();
}