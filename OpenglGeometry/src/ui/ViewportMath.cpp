#include "ViewportMath.h"

#include <cmath>

#include "interfaces/ICamera.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

std::optional<GUI::ViewportCamera> GUI::ActiveViewportCamera(Ref<Scene> scene)
{
	for (Entity entity : scene->GetAllEntitiesWith<CameraComponent>())
	{
		const auto& cameraComponent = entity.GetComponent<CameraComponent>();
		if (!cameraComponent.active || !cameraComponent.cameraHandling)
		{
			continue;
		}

		ViewportCamera camera;
		camera.view = cameraComponent.viewMatrix;
		camera.projection = cameraComponent.projectionMatrix;
		camera.position = cameraComponent.cameraHandling->GetPosition();
		camera.right = cameraComponent.cameraHandling->GetRight();
		camera.up = cameraComponent.cameraHandling->GetUp();
		camera.forward = cameraComponent.cameraHandling->GetForward();

		return camera;
	}

	return std::nullopt;
}

bool GUI::ProjectToViewport(const ViewportCamera& camera, const Algebra::Vector4& world,
	const ImVec2& rectMin, const ImVec2& rectMax, ImVec2& outScreen)
{
	const Algebra::Vector4 clip = camera.projection * (camera.view
		* Algebra::Vector4(world.x, world.y, world.z, 1.f));

	if (clip.w <= 1e-4f)
	{
		return false;
	}

	const float ndcX = clip.x / clip.w;
	const float ndcY = clip.y / clip.w;

	outScreen = ImVec2(
		rectMin.x + (ndcX * 0.5f + 0.5f) * (rectMax.x - rectMin.x),
		rectMin.y + (0.5f - ndcY * 0.5f) * (rectMax.y - rectMin.y));

	return true;
}

Algebra::Vector4 GUI::ViewportRayDirection(const ViewportCamera& camera, const ImVec2& screen,
	const ImVec2& rectMin, const ImVec2& rectMax)
{
	const float ndcX = 2.f * (screen.x - rectMin.x) / (rectMax.x - rectMin.x) - 1.f;
	const float ndcY = 1.f - 2.f * (screen.y - rectMin.y) / (rectMax.y - rectMin.y);

	const float tanX = 1.f / camera.projection[0][0];
	const float tanY = 1.f / camera.projection[1][1];

	const Algebra::Vector4 direction = camera.forward
		+ camera.right * (ndcX * tanX)
		+ camera.up * (ndcY * tanY);

	return direction.Normalize();
}

bool GUI::ViewportRayPlaneHit(const ViewportCamera& camera, const ImVec2& screen,
	const ImVec2& rectMin, const ImVec2& rectMax,
	const Algebra::Vector4& planePoint, const Algebra::Vector4& planeNormal,
	Algebra::Vector4& outWorld)
{
	const Algebra::Vector4 direction = ViewportRayDirection(camera, screen, rectMin, rectMax);

	const float denominator = direction * planeNormal;
	if (std::abs(denominator) < 1e-6f)
	{
		return false;
	}

	const float t = ((planePoint - camera.position) * planeNormal) / denominator;
	if (t <= 0.f)
	{
		return false;
	}

	outWorld = camera.position + direction * t;
	outWorld.w = 1.f;
	return true;
}
