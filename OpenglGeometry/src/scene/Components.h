#pragma once
#include <vector>
#include "core/Base.h"
#include "renderer/VertexArray.h"
#include "managers/ShaderManager.h"
#include "Algebra.h"
#include "Tags.h"
#include "renderer/Renderer.h"
#include "managers/IdManager.h"

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

struct MeshComponent
{
	Ref<VertexArray> mesh;
	AvailableShaders shaderType = AvailableShaders::Default;
	RenderingMode renderingMode = RenderingMode::Triangles;

	MeshComponent() = default;
	MeshComponent(const MeshComponent& other) = default;
};

struct TorusGenerationComponent
{
	float radius = 1.0f;
	float tubeRadius = 0.2f;
	unsigned int radialSegments = 16;
	unsigned int tubularSegments = 32;
	TorusGenerationComponent() = default;
	TorusGenerationComponent(const TorusGenerationComponent& other) = default;
};

struct IdComponent
{
	const ID id = IdManager::GetInstance().GetNewId();
	IdComponent() = default;
	IdComponent(const IdComponent& other) = default;

	~IdComponent()
	{
		IdManager::GetInstance().FreeId(id);
	}
};

struct NameComponent
{
	std::string name;
	NameComponent() = default;
	NameComponent(const NameComponent& other) = default;
};
