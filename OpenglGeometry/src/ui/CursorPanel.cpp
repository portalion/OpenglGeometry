#include "CursorPanel.h"

#include <array>
#include <cstddef>
#include <optional>

#include <imgui/imgui.h>

#include "Algebra.h"
#include "Utils.h"
#include "Widgets.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "scene/Tags.h"
#include "ui/ViewportMath.h"
#include "ui/core/DockSpace.h"

namespace
{
	Entity FindCursorEntity(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<CursorTag, PositionComponent>())
		{
			return entity;
		}

		return Entity{};
	}

	struct SelectionSummary
	{
		Algebra::Vector4 centre = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
		std::size_t      count = 0;
	};

	SelectionSummary SummariseSelection(Ref<Scene> scene)
	{
		Algebra::Vector4 sum(0.f, 0.f, 0.f, 0.f);
		std::size_t count = 0;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>())
		{
			const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;
			sum = sum + Algebra::Vector4(position.x, position.y, position.z, 0.f);
			count++;
		}

		SelectionSummary result;
		result.count = count;

		if (count > 0)
		{
			const Algebra::Vector4 average = sum / static_cast<float>(count);
			result.centre = Algebra::Vector4(average.x, average.y, average.z, 1.f);
		}

		return result;
	}
}

void GUI::DrawCursorPanel(UiState& state)
{
	ImGui::Begin(CursorPanelWindow);

	CursorState& cursor = state.cursor;

	if (BeginPropertyTable("##CursorCoordinates"))
	{
		PropertyRow("World", cursor.world);
		PropertyRow("Screen X", cursor.screenX, 1.0f);
		PropertyRow("Screen Y", cursor.screenY, 1.0f);
		EndPropertyTable();
	}

	ImGui::TextDisabled("screen unprojected onto the plane through the cursor - phase 5");

	ImGui::Separator();

	static constexpr std::array<const char*, 2> onClickOptions = { "Keep depth", "Snap to nearest object" };
	int onClickIndex = cursor.snapToNearest ? 1 : 0;

	ImGui::TextUnformatted("On click");
	ImGui::SameLine();
	if (SegmentedControl("##OnClick", onClickIndex, onClickOptions))
	{
		cursor.snapToNearest = (onClickIndex == 1);
	}

	if (BeginPropertyTable("##CursorCentre"))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Centre");
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginDisabled();
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat3("##Centre", &cursor.selectionCentre.x);
		ImGui::EndDisabled();
		EndPropertyTable();
	}

	ImGui::TextDisabled("selection centre - read-only - %zu objects", state.SelectedCount());

	ImGui::Separator();

	if (ImGui::Button("Cursor -> centre"))
	{
		cursor.world = cursor.selectionCentre;
	}

	ImGui::SameLine();

	if (ImGui::Button("Cursor -> origin"))
	{
		cursor.world = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
	}

	ImGui::End();
}

void GUI::DrawCursorPanel(Ref<Scene> scene, UiState& state, const Dockspace& dockspace)
{
	CursorState& cursor = state.cursor;

	Entity cursorEntity = FindCursorEntity(scene);
	if (cursorEntity.IsValid())
	{
		cursor.world = cursorEntity.GetComponent<PositionComponent>().position;
	}

	const SelectionSummary selection = SummariseSelection(scene);
	cursor.selectionCentre = selection.centre;

	ImVec2 rectMin;
	ImVec2 rectMax;
	const bool haveViewport = dockspace.TryGetCentralNodeScreenRect(rectMin, rectMax);
	const std::optional<ViewportCamera> camera =
		haveViewport ? ActiveViewportCamera(scene) : std::nullopt;

	bool cursorOnScreen = false;
	if (camera.has_value())
	{
		ImVec2 projected;
		if (ProjectToViewport(*camera, cursor.world, rectMin, rectMax, projected))
		{
			cursor.screenX = projected.x - rectMin.x;
			cursor.screenY = projected.y - rectMin.y;
			cursorOnScreen = true;
		}
	}

	const Algebra::Vector4 worldBefore = cursor.world;

	ImGui::Begin(CursorPanelWindow);

	if (BeginPropertyTable("##CursorCoordinates"))
	{
		PropertyRow("World", cursor.world);

		ImGui::BeginDisabled(!cursorOnScreen);
		bool screenChanged = PropertyRow("Screen X", cursor.screenX, 1.0f);
		screenChanged = PropertyRow("Screen Y", cursor.screenY, 1.0f) || screenChanged;
		ImGui::EndDisabled();

		if (screenChanged && cursorOnScreen)
		{
			const ImVec2 target(rectMin.x + cursor.screenX, rectMin.y + cursor.screenY);
			Algebra::Vector4 hit;
			if (ViewportRayPlaneHit(*camera, target, rectMin, rectMax, worldBefore, camera->forward, hit))
			{
				cursor.world = hit;
			}
		}

		EndPropertyTable();
	}

	ImGui::TextDisabled(cursorOnScreen
		? "screen pixels within the viewport - editable"
		: "cursor is off screen");

	ImGui::Separator();

	static constexpr std::array<const char*, 2> onClickOptions = { "Keep depth", "Snap to nearest object" };
	int onClickIndex = cursor.snapToNearest ? 1 : 0;

	ImGui::TextUnformatted("On right-click");
	ImGui::SameLine();
	if (SegmentedControl("##OnClick", onClickIndex, onClickOptions))
	{
		cursor.snapToNearest = (onClickIndex == 1);
	}

	ImGui::TextDisabled("right-click in the viewport to place the cursor");

	if (BeginPropertyTable("##CursorCentre"))
	{
		ImGui::BeginDisabled();
		PropertyRow("Centre", cursor.selectionCentre);
		ImGui::EndDisabled();
		EndPropertyTable();
	}

	ImGui::TextDisabled("selection centre - read-only - %zu objects", selection.count);

	ImGui::Separator();

	ImGui::BeginDisabled(selection.count == 0);
	if (ImGui::Button("Cursor -> centre"))
	{
		cursor.world = cursor.selectionCentre;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Cursor -> origin"))
	{
		cursor.world = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
	}

	ImGui::End();

	if (cursorEntity.IsValid() && !(cursor.world == worldBefore))
	{
		cursorEntity.GetComponent<PositionComponent>().position = cursor.world;
	}
}
