#include "CursorPanel.h"

#include <array>
#include <imgui/imgui.h>

#include "Utils.h"
#include "Widgets.h"

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
