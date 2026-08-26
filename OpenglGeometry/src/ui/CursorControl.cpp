#include "CursorControl.h"

#include <cmath>
#include <optional>

#include <imgui/imgui.h>

#include "Algebra.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "scene/Tags.h"
#include "ui/ViewportMath.h"
#include "ui/core/DockSpace.h"
#include "ui/model/UiModel.h"

namespace
{
	Entity FindCursor(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<CursorTag, PositionComponent>())
		{
			return entity;
		}

		return Entity{};
	}

	Entity NearestObject(Ref<Scene> scene, const GUI::ViewportCamera& camera, const ImVec2& screen,
		const ImVec2& rectMin, const ImVec2& rectMax, float radius)
	{
		Entity nearest;
		float nearestDistance = radius;

		for (Entity entity : scene->GetAllEntitiesWith<ObjectTypeComponent, PositionComponent>())
		{
			const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;

			ImVec2 projected;
			if (!GUI::ProjectToViewport(camera, position, rectMin, rectMax, projected))
			{
				continue;
			}

			const float distance = std::hypot(projected.x - screen.x, projected.y - screen.y);
			if (distance < nearestDistance)
			{
				nearestDistance = distance;
				nearest = entity;
			}
		}

		return nearest;
	}

	void MoveCursorTo(Entity cursor, const Algebra::Vector4& worldPosition)
	{
		cursor.GetComponent<PositionComponent>().position =
			Algebra::Vector4(worldPosition.x, worldPosition.y, worldPosition.z, 1.f);
	}
}

void GUI::HandleCursorPlacement(Ref<Scene> scene, const Dockspace& dockspace,
	const CursorState& cursorState, CursorPlacementState& state)
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

	if (!state.tracking
		&& ImGui::IsMouseClicked(ImGuiMouseButton_Right)
		&& mouseInViewport
		&& !io.WantCaptureMouse)
	{
		state.tracking = true;
		state.pressPos = mouse;
	}

	if (!state.tracking)
	{
		return;
	}

	const float travel = std::hypot(mouse.x - state.pressPos.x, mouse.y - state.pressPos.y);
	if (travel > io.MouseDragThreshold)
	{
		state.tracking = false; 
		return;
	}

	if (!ImGui::IsMouseReleased(ImGuiMouseButton_Right))
	{
		return;
	}

	state.tracking = false;

	Entity cursor = FindCursor(scene);
	if (!cursor.IsValid())
	{
		return;
	}

	const std::optional<ViewportCamera> camera = ActiveViewportCamera(scene);
	if (!camera.has_value() || std::abs(camera->projection[1][1]) < 1e-6f)
	{
		return;
	}

	if (cursorState.snapToNearest)
	{
		Entity nearest = NearestObject(scene, *camera, state.pressPos, rectMin, rectMax, 16.f);
		if (nearest.IsValid())
		{
			MoveCursorTo(cursor, nearest.GetComponent<PositionComponent>().position);
			return;
		}
	}

	const Algebra::Vector4 planePoint = cursor.GetComponent<PositionComponent>().position;

	Algebra::Vector4 hit;
	if (ViewportRayPlaneHit(*camera, state.pressPos, rectMin, rectMax, planePoint, camera->forward, hit))
	{
		MoveCursorTo(cursor, hit);
	}
}
