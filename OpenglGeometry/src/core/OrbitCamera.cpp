#include "OrbitCamera.h"
#include "core/Globals.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>

namespace
{
	constexpr float rotationSensitivity = 3.f;
	constexpr float zoomStep = 0.9f;
	constexpr float minDistance = 0.5f;
	constexpr float maxDistance = 500.f;
	constexpr float maxPitch = 1.56f;
}

OrbitCamera::OrbitCamera(Algebra::Vector4 target, float distance, float pitch, float yaw)
	: target{ target }, distance{ std::clamp(distance, minDistance, maxDistance) },
	yaw{ yaw }, pitch{ std::clamp(pitch, -maxPitch, maxPitch) }, roll{ 0.f }
{
}

Algebra::Matrix4 OrbitCamera::GetRotationMatrix() const
{
	return Algebra::Matrix4::RotationZ(roll)
		* Algebra::Matrix4::RotationX(pitch)
		* Algebra::Matrix4::RotationY(yaw);
}

Algebra::Matrix4 OrbitCamera::GetViewMatrix() const
{
	return Algebra::Matrix4::Translation(0.f, 0.f, -distance)
		* GetRotationMatrix()
		* Algebra::Matrix4::Translation(-target.x, -target.y, -target.z);
}

Algebra::Vector4 OrbitCamera::GetPosition() const
{
	return target + GetRotationMatrix().Transpose() * Algebra::Vector4(0.f, 0.f, distance, 0.f);
}

Algebra::Vector4 OrbitCamera::GetRight() const
{
	return GetRotationMatrix().Transpose() * Algebra::Vector4(1.f, 0.f, 0.f, 0.f);
}

Algebra::Vector4 OrbitCamera::GetUp() const
{
	return GetRotationMatrix().Transpose() * Algebra::Vector4(0.f, 1.f, 0.f, 0.f);
}

void OrbitCamera::Focus(const Algebra::Vector4& point, float radius)
{
	target = Algebra::Vector4(point.x, point.y, point.z, 1.f);

	if (radius > 0.f)
	{
		const float fitDistance = 1.2f * radius / std::sin(Globals::cameraFieldOfView / 2.f);
		distance = std::clamp(fitDistance, minDistance, maxDistance);
	}
}

void OrbitCamera::HandlePan(const ViewportData& viewport)
{
	if (!ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		return;
	}

	const ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);

	const float height = static_cast<float>(std::max(viewport.height, 1));
	const float worldPerPixel = 2.f * distance * std::tan(Globals::cameraFieldOfView / 2.f) / height;

	target = target
		- GetRight() * (delta.x * worldPerPixel)
		+ GetUp() * (delta.y * worldPerPixel);
}

void OrbitCamera::HandleZoom()
{
	const float wheel = ImGui::GetIO().MouseWheel;
	if (wheel == 0.f)
	{
		return;
	}

	distance = std::clamp(distance * std::pow(zoomStep, wheel), minDistance, maxDistance);
}

void OrbitCamera::HandleRotations(const ViewportData& viewport)
{
	if (!ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		return;
	}

	const float width = static_cast<float>(std::max(viewport.width, 1));
	const float height = static_cast<float>(std::max(viewport.height, 1));

	const ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

	if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
	{
		roll -= delta.x / width * rotationSensitivity;
		return;
	}

	yaw -= delta.x / width * rotationSensitivity;
	pitch = std::clamp(pitch - delta.y / height * rotationSensitivity, -maxPitch, maxPitch);
}

void OrbitCamera::HandleInput(CameraComponent& cameraComponent, const ViewportData& viewport)
{
	HandlePan(viewport);
	HandleZoom();
	HandleRotations(viewport);

	cameraComponent.viewMatrix = GetViewMatrix();
}
