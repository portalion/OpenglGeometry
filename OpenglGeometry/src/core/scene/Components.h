#pragma once
#include "engine/Renderable.h"

struct MeshComponent
{
	RenderableMesh<PositionVertexData> mesh;

	MeshComponent() = default;
	MeshComponent(const MeshComponent& other) = default;
};

struct PositionComponent
{
	Algebra::Vector4 position;

	PositionComponent() = default;
	PositionComponent(const PositionComponent& other) = default;

	void Move(Algebra::Vector4 offset)
	{
		position += offset;
	}
};

struct ScaleComponent
{
	Algebra::Vector4 scale;

	ScaleComponent() = default;
	ScaleComponent(const ScaleComponent& other) = default;
};

struct RotationComponent
{
	Algebra::Quaternion rotation;

	RotationComponent() = default;
	RotationComponent(const RotationComponent& other) = default;

	void Rotate(Algebra::Quaternion rotation)
	{
		this->rotation = this->rotation * rotation;
		this->rotation.Normalize();
	}
};

struct CameraComponent
{
	Algebra::Matrix4 projectionMatrix;

	CameraComponent() = default;
	CameraComponent(const CameraComponent& other) = default;
};
