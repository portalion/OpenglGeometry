#pragma once
#include <vector>
#include "core/Base.h"
#include "renderer/VertexArray.h"
#include "managers/ShaderManager.h"
#include "Algebra.h"

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
	bool active = false;
	Algebra::Matrix4 projectionMatrix;

	CameraComponent() = default;
	CameraComponent(const CameraComponent& other) = default;
};

struct StaticMeshComponent
{
	Ref<VertexArray> mesh;
	AvailableShaders shaderType = AvailableShaders::Default;

	StaticMeshComponent() = default;
	StaticMeshComponent(const StaticMeshComponent& other) = default;
};
