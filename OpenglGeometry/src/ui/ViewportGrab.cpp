#include "ViewportGrab.h"

#include <optional>

#include <imgui/imgui.h>

#include "Algebra.h"
#include "scene/Scene.h"
#include "ui/SceneActions.h"
#include "ui/ViewportMath.h"
#include "ui/core/DockSpace.h"

namespace
{
	void ApplyTranslation(Ref<Scene> scene, const Algebra::Vector4& delta)
	{
		GUI::SelectionTransform transform;
		transform.translation = Algebra::Vector4(delta.x, delta.y, delta.z, 0.f);
		GUI::ApplySelectionTransform(scene, GUI::TransformSpace::Centre, transform);
	}
}

bool GUI::HandleViewportGrab(Ref<Scene> scene, const Dockspace& dockspace, ViewportGrabState& state)
{
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 rectMin;
	ImVec2 rectMax;
	if (!dockspace.TryGetCentralNodeScreenRect(rectMin, rectMax))
	{
		return false;
	}

	const ImVec2 mouse = io.MousePos;
	const bool mouseInViewport =
		mouse.x >= rectMin.x && mouse.x < rectMax.x &&
		mouse.y >= rectMin.y && mouse.y < rectMax.y;

	if (!state.active)
	{
		if (!io.WantTextInput
			&& mouseInViewport
			&& !io.WantCaptureMouse
			&& AnythingSelected(scene)
			&& ImGui::Shortcut(ImGuiKey_G, ImGuiInputFlags_RouteGlobal))
		{
			state.active = true;
			state.hasLastHit = false;
			state.totalDelta = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			state.planePoint = SelectionCentre(scene);
		}
		return false;
	}

	if (!AnythingSelected(scene))
	{
		state.active = false;
		return false;
	}

	const std::optional<ViewportCamera> camera = ActiveViewportCamera(scene);
	if (!camera.has_value())
	{
		state.active = false;
		return false;
	}

	const bool cancel = ImGui::IsKeyPressed(ImGuiKey_Escape)
		|| ImGui::IsMouseClicked(ImGuiMouseButton_Right);

	if (cancel)
	{
		ApplyTranslation(scene, Algebra::Vector4(
			-state.totalDelta.x, -state.totalDelta.y, -state.totalDelta.z, 0.f));
		state.active = false;
		return true;
	}

	Algebra::Vector4 hit;
	if (ViewportRayPlaneHit(*camera, mouse, rectMin, rectMax, state.planePoint, camera->forward, hit))
	{
		if (state.hasLastHit)
		{
			const Algebra::Vector4 delta(
				hit.x - state.lastHit.x, hit.y - state.lastHit.y, hit.z - state.lastHit.z, 0.f);
			ApplyTranslation(scene, delta);
			state.totalDelta = state.totalDelta + delta;
		}
		state.lastHit = hit;
		state.hasLastHit = true;
	}

	const bool confirm = ImGui::IsMouseClicked(ImGuiMouseButton_Left)
		|| ImGui::IsKeyPressed(ImGuiKey_Enter)
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);

	if (confirm)
	{
		state.active = false;
		return true;
	}

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	drawList->AddText(ImVec2(rectMin.x + 12.f, rectMin.y + 12.f), IM_COL32(255, 220, 120, 255),
		"Grab: move mouse to translate  -  LMB / Enter confirm  -  Esc / RMB cancel");

	return true;
}
