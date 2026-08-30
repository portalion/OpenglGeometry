#pragma once
#include <vector>
#include <list>
#include "core/Base.h"
#include "renderer/VertexArray.h"
#include "managers/ShaderManager.h"
#include "Algebra.h"
#include "Tags.h"
#include "renderer/Renderer.h"
#include "managers/IdManager.h"
#include "core/Globals.h"
#include "Observable.h"
#include "ObjectType.h"

struct PositionComponent
{
	Observable<Algebra::Vector4> position;

	PositionComponent() = default;
	PositionComponent(const PositionComponent& other) = default;

	void Move(Algebra::Vector4 offset)
	{
		position = offset;
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

struct ColorComponent
{
	Algebra::Vector4 color = Globals::defaultPointsColor;

	ColorComponent() = default;
	ColorComponent(const ColorComponent& other) = default;
};

class ICamera;

struct CameraComponent
{
	bool active = false;
	Algebra::Matrix4 projectionMatrix;
	Algebra::Matrix4 viewMatrix;
	Ref<ICamera> cameraHandling;

	bool stereoscopic = false;
	float eyeDistance = 0.1f;
	float convergence = 10.f;
	Algebra::Vector4 leftEyeColor = Algebra::Vector4(0.90f, 0.28f, 0.28f, 1.f);
	Algebra::Vector4 rightEyeColor = Algebra::Vector4(0.17f, 0.83f, 0.83f, 1.f);

	CameraComponent() = default;
	CameraComponent(const CameraComponent& other) = default;
};

struct MeshComponent
{
	Ref<VertexArray> mesh;
	std::vector<AvailableShaders> shaderTypes;
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
	ID id = IdManager::GetInstance().GetNewId();

	IdComponent() = default;

	IdComponent(ID id) : id(id) { }

	IdComponent(const IdComponent& other) = delete;
	IdComponent& operator=(const IdComponent& other) = delete;
	IdComponent(IdComponent&& other) = default;
	IdComponent& operator=(IdComponent&& other) = default;
};

struct NameComponent
{
	std::string name;
	NameComponent() = default;
	NameComponent(const NameComponent& other) = default;
};

struct NotificationComponent
{
	std::list<Entity> entitiesToNotify;
	NotificationComponent() = default;
	NotificationComponent(const NotificationComponent& other) = default;
};

struct LineGenerationComponent
{
	std::list<Entity> controlPoints;
	LineGenerationComponent() = default;
	LineGenerationComponent(const LineGenerationComponent& other) = default;
};

struct VirtualEntityComponent
{
	Entity realEntity;
	VirtualEntityComponent() = default;
	VirtualEntityComponent(const VirtualEntityComponent& other) = default;
};

struct IsParentOfVirtualEntitiesComponent
{
	std::vector<Entity> virtualEntities;
	IsParentOfVirtualEntitiesComponent() = default;
	IsParentOfVirtualEntitiesComponent(const IsParentOfVirtualEntitiesComponent& other) = default;
};

struct BezierLineGenerationComponent
{
	std::function<std::vector<Algebra::Vector4>(const std::vector<Algebra::Vector4>&)> generationFunction;
	BezierLineGenerationComponent() = default;
	BezierLineGenerationComponent(const BezierLineGenerationComponent& other) = default;
};

struct CurveHelpersComponent
{
	Entity controlPolyline;
	std::vector<Entity> bernsteinPoints;
	Entity bernsteinPolyline;
	bool showBernstein = false;
	CurveHelpersComponent() = default;
	CurveHelpersComponent(const CurveHelpersComponent& other) = default;
};

struct BernsteinPointComponent
{
	Entity curve;
	std::size_t deBoorIndex = 0;
	Algebra::Vector4 lastComputed;
	BernsteinPointComponent() = default;
	BernsteinPointComponent(const BernsteinPointComponent& other) = default;
};

struct BezierPatchGenerationComponent
{
	std::array<std::array<Entity, 4>, 4> controlPoints;
	BezierPatchGenerationComponent() = default;
	BezierPatchGenerationComponent(const BezierPatchGenerationComponent& other) = default;
};

struct BezierSurfaceGenerationComponent
{
	std::vector<std::vector<Entity>> bezierPatches;
	int samplesU = 32;
	int samplesV = 32;
	BezierSurfaceGenerationComponent() = default;
	BezierSurfaceGenerationComponent(const BezierSurfaceGenerationComponent& other) = default;
};

struct SurfaceControlNetComponent
{
	std::vector<std::vector<Entity>> grid;
	SurfaceControlNetComponent() = default;
	SurfaceControlNetComponent(const SurfaceControlNetComponent& other) = default;
};
