#include "Camera.h"
#include <imgui/imgui.h>
#include <algorithm>

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
	return Algebra::Matrix4::RotationXDegree(xAngle) * Algebra::Matrix4::RotationYDegree(yAngle);
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

		position += direction * 1000.f * dt * GetZoomMatrix();
	}
}

void Camera::HandleZoom(const float& dt)
{
	if (ImGui::GetIO().MouseWheel != 0.f)
	{
		zoom += ImGui::GetIO().MouseWheel * 0.1f;
		zoom = std::clamp(zoom, 0.1f, 5.0f);
	}
}

void Camera::HandleRotations(const float& dt)
{
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
		yAngle += delta.x / 10.f;
		xAngle += delta.y / 10.f;
		
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

		xAngle = std::clamp(xAngle, 0.f, 180.f);
		if (yAngle > 360)
		{
			yAngle = 0;
		}
		else if(yAngle < 0)
		{
			yAngle = 360;
		}
	}
}

Camera::Camera(Algebra::Vector4 position, float zoom)
	:position{ position }, zoom{ zoom }, yAngle{ 0 }, xAngle{ 0 }
{
}

void Camera::HandleInput()
{
	const float dt = ImGui::GetIO().DeltaTime;
	HandleTranslation(dt);
	HandleZoom(dt);
	HandleRotations(dt);
}
