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
	using namespace GUI;

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

	bool SectionHeader(const char* label)
	{
		return ImGui::CollapsingHeader(label,
			ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
	}

	void DrawTransformSelectionBlock(UiState& state, const GUI::CursorPanelCallbacks* callbacks)
	{
		if (!SectionHeader("TRANSFORM THE SELECTION"))
		{
			return;
		}

		static constexpr std::array<const char*, 3> pivotOptions = { "Own origin", "Centre", "3D cursor" };
		int pivotIndex = static_cast<int>(state.pivot);

		ImGui::TextUnformatted("Pivot");
		ImGui::SameLine();
		if (SegmentedControl("##MultiPivot", pivotIndex, pivotOptions))
		{
			state.pivot = static_cast<PivotMode>(pivotIndex);
		}

		if (state.pivot == PivotMode::Origin)
		{
			ImGui::TextDisabled("each object in its own local axes");
		}
		else if (state.pivot == PivotMode::Cursor)
		{
			const Algebra::Vector4 c = state.cursor.world;
			ImGui::TextDisabled("pivot: 3D cursor  %.3f, %.3f, %.3f", c.x, c.y, c.z);
		}
		else
		{
			const Algebra::Vector4 c = state.cursor.selectionCentre;
			ImGui::TextDisabled("pivot: selection centre  %.3f, %.3f, %.3f", c.x, c.y, c.z);
		}

		static Algebra::Vector4 s_MoveBy(0.f, 0.f, 0.f, 0.f);
		static Algebra::Vector4 s_RotateBy(0.f, 0.f, 0.f, 0.f);
		static Algebra::Vector4 s_ScaleBy(1.f, 1.f, 1.f, 0.f);

		if (BeginPropertyTable("##MultiTransform"))
		{
			PropertyRow("Move by", s_MoveBy);
			PropertyRow("Rotate by", s_RotateBy);
			PropertyRow("Scale by", s_ScaleBy);
			EndPropertyTable();
		}

		const bool canApply = callbacks && callbacks->applySelectionTransform;

		ImGui::BeginDisabled(!canApply);
		if (ImGui::Button("Apply") && canApply)
		{
			callbacks->applySelectionTransform(state.pivot, s_MoveBy, s_RotateBy, s_ScaleBy);
			s_MoveBy = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			s_RotateBy = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			s_ScaleBy = Algebra::Vector4(1.f, 1.f, 1.f, 0.f);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Reset fields"))
		{
			s_MoveBy = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			s_RotateBy = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
			s_ScaleBy = Algebra::Vector4(1.f, 1.f, 1.f, 0.f);
		}
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

	ImGui::Separator();

	DrawTransformSelectionBlock(state, nullptr);

	ImGui::End();
}

void GUI::DrawCursorPanel(Ref<Scene> scene, UiState& state, const Dockspace& dockspace,
	const CursorPanelCallbacks* callbacks)
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

	ImGui::Separator();

	DrawTransformSelectionBlock(state, callbacks);

	ImGui::End();

	if (cursorEntity.IsValid() && !(cursor.world == worldBefore))
	{
		cursorEntity.GetComponent<PositionComponent>().position = cursor.world;
	}
}
