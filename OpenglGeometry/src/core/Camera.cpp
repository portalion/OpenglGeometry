#include "Camera.h"
#include <imgui/imgui.h>
#include <algorithm>
#include "Globals.h"

Algebra::Matrix4 Camera::GetTranslationMatrix()
{
	return Algebra::Matrix4::Translation(-position.x, -position.y, -position.z);
}

Algebra::Matrix4 Camera::GetZoomMatrix()
{
	return Algebra::Matrix4::DiagonalScaling(zoom, zoom, zoom);
}

Algebra::Matrix4 Camera::GetRotationMatrix()
{
	return rotation.ToMatrix();
}

void Camera::HandleTranslation(const float& dt)
{
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);

		Algebra::Vector4 direction = Algebra::Vector4(delta.x, -delta.y, 0, 0);
		if (direction.Length() > 0)
		{
			direction = direction.Normalize();
		}

		position += direction * 100.f * dt * GetRotationMatrix() * GetZoomMatrix();
	}
}

void Camera::HandleZoom(const float& dt)
{
	if (ImGui::GetIO().MouseWheel != 0.f)
	{
		zoom *= ImGui::GetIO().MouseWheel > 0 ? 1.1f : 1.f/1.1f;
	}
}

void Camera::HandleRotations(const float& dt)
{
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
		{
			ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			float rollDelta = delta.x / Globals::startingSceneWidth * 3.f;

			Algebra::Vector4 forward = rotation.Rotate(Algebra::Vector4(0, 0, -1, 0));

			Algebra::Quaternion rollQuat = Algebra::Quaternion::CreateFromAxisAngle(forward, rollDelta);
			rotation = (rollQuat * rotation).Normalize();

			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
			return;
		}

		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
		float yawDelta = delta.x / Globals::startingSceneWidth * 3.f;
		float pitchDelta = delta.y / Globals::startingSceneHeight * 3.f;

		Algebra::Quaternion yawQuat = Algebra::Quaternion::CreateFromAxisAngle(Algebra::Vector4(0, 1, 0, 0), -yawDelta);

		Algebra::Quaternion tempRotation = (yawQuat * rotation).Normalize();

		Algebra::Vector4 right = tempRotation.Rotate(Algebra::Vector4(1, 0, 0, 0));

		Algebra::Quaternion pitchQuat = Algebra::Quaternion::CreateFromAxisAngle(right, -pitchDelta);

		rotation = (pitchQuat * tempRotation).Normalize();

		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
	}
}

Camera::Camera(Algebra::Vector4 position, float zoom)
	:position{ position }, zoom{ zoom }, rotation{1, 0, 0, 0}
{
}

void Camera::HandleInput()
{
	const float dt = ImGui::GetIO().DeltaTime;
	HandleTranslation(dt);
	HandleZoom(dt);
	HandleRotations(dt);
}
