#include "ViewportPicking.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include <imgui/imgui.h>

#include "Algebra.h"
#include "core/Globals.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "scene/Tags.h"
#include "ui/SceneActions.h"
#include "ui/core/DockSpace.h"

namespace
{
	constexpr float c_clickMoveThreshold = 5.f;   
	constexpr float c_clickPickRadius = 12.f;      

	struct ActiveCamera
	{
		Algebra::Matrix4 view;
		Algebra::Matrix4 projection;
	};

	std::optional<ActiveCamera> FindActiveCamera(Ref<Scene> scene)
	{
		for (Entity camera : scene->GetAllEntitiesWith<CameraComponent>())
		{
			const auto& cameraComponent = camera.GetComponent<CameraComponent>();
			if (!cameraComponent.active)
			{
				continue;
			}

			return ActiveCamera{ cameraComponent.viewMatrix, cameraComponent.projectionMatrix };
		}

		return std::nullopt;
	}

	struct ScreenPoint
	{
		Entity entity;
		ImVec2 pos;
		float  depth;  
	};

	std::vector<ScreenPoint> ProjectPoints(Ref<Scene> scene, const ActiveCamera& camera,
		const ImVec2& rectMin, const ImVec2& rectMax)
	{
		const float width = rectMax.x - rectMin.x;
		const float height = rectMax.y - rectMin.y;

		std::vector<ScreenPoint> result;

		for (Entity entity : scene->GetAllEntitiesWith<ObjectTypeComponent, PositionComponent>())
		{
			if (entity.GetComponent<ObjectTypeComponent>().type != ObjectType::Point)
			{
				continue;
			}

			const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;
			const Algebra::Vector4 clip = camera.projection * (camera.view
				* Algebra::Vector4(position.x, position.y, position.z, 1.f));

			if (clip.w <= 1e-4f)
			{
				continue;
			}

			const float ndcX = clip.x / clip.w;
			const float ndcY = clip.y / clip.w;

			result.push_back(ScreenPoint{
				entity,
				ImVec2(rectMin.x + (ndcX * 0.5f + 0.5f) * width,
					   rectMin.y + (0.5f - ndcY * 0.5f) * height),
				clip.w });
		}

		return result;
	}

	void Toggle(Entity entity)
	{
		if (entity.HasComponent<IsSelectedTag>())
		{
			entity.RemoveTag<IsSelectedTag>();
		}
		else
		{
			entity.AddTag<IsSelectedTag>();
		}
	}
}

void GUI::HandleViewportPicking(Ref<Scene> scene, const Dockspace& dockspace, ViewportPickingState& state)
{
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 rectMin;
	ImVec2 rectMax;
	if (!dockspace.TryGetCentralNodeScreenRect(rectMin, rectMax))
	{
		return;
	}

	const ImVec2 mouse = io.MousePos;
	const bool mouseInViewport =
		mouse.x >= rectMin.x && mouse.x < rectMax.x &&
		mouse.y >= rectMin.y && mouse.y < rectMax.y;

	if (!state.dragging
		&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
		&& mouseInViewport
		&& !io.WantCaptureMouse
		&& !ImGui::IsMouseDown(ImGuiMouseButton_Right)
		&& !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	{
		state.dragging = true;
		state.dragStart = mouse;
	}

	if (!state.dragging)
	{
		return;
	}

	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		state.dragging = false;
		return;
	}

	const auto clampToViewport = [&](ImVec2 point)
	{
		return ImVec2(std::clamp(point.x, rectMin.x, rectMax.x),
			std::clamp(point.y, rectMin.y, rectMax.y));
	};

	const ImVec2 boxMin = clampToViewport(ImVec2(std::min(state.dragStart.x, mouse.x), std::min(state.dragStart.y, mouse.y)));
	const ImVec2 boxMax = clampToViewport(ImVec2(std::max(state.dragStart.x, mouse.x), std::max(state.dragStart.y, mouse.y)));

	const float travel = std::hypot(mouse.x - state.dragStart.x, mouse.y - state.dragStart.y);
	const bool isBox = travel > c_clickMoveThreshold;

	if (isBox && !ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		drawList->AddRectFilled(boxMin, boxMax, IM_COL32(120, 170, 255, 40));
		drawList->AddRect(boxMin, boxMax, IM_COL32(120, 170, 255, 220));
	}

	if (!ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		return;
	}

	state.dragging = false;

	const std::optional<ActiveCamera> camera = FindActiveCamera(scene);
	if (!camera.has_value())
	{
		return;
	}

	const bool additive = io.KeyShift || io.KeyCtrl;
	const std::vector<ScreenPoint> points = ProjectPoints(scene, *camera, rectMin, rectMax);

	if (isBox)
	{
		if (!additive)
		{
			GUI::DeselectAll(scene);
		}

		for (const ScreenPoint& point : points)
		{
			if (point.pos.x >= boxMin.x && point.pos.x <= boxMax.x &&
				point.pos.y >= boxMin.y && point.pos.y <= boxMax.y)
			{
				Entity entity = point.entity;
				entity.AddTag<IsSelectedTag>();
			}
		}

		return;
	}

	Entity hit;
	float bestDistance = c_clickPickRadius;
	float bestDepth = std::numeric_limits<float>::max();

	for (const ScreenPoint& point : points)
	{
		const float distance = std::hypot(point.pos.x - state.dragStart.x, point.pos.y - state.dragStart.y);
		if (distance > c_clickPickRadius)
		{
			continue;
		}

		if (distance < bestDistance - 1.f || (distance <= bestDistance + 1.f && point.depth < bestDepth))
		{
			hit = point.entity;
			bestDistance = std::min(bestDistance, distance);
			bestDepth = point.depth;
		}
	}

	if (hit.IsValid())
	{
		if (additive)
		{
			Toggle(hit);
		}
		else
		{
			GUI::SelectOnly(scene, hit);
		}
	}
	else if (!additive)
	{
		GUI::DeselectAll(scene);
	}
}
