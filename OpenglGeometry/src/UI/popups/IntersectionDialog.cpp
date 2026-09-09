#include "IntersectionDialog.h"

#include <algorithm>
#include <imgui/imgui.h>

#include "ui/Widgets.h"
#include "ui/Intersections.h"
#include "geometry/IntersectionFinder.h"

namespace
{
	struct IntersectionDraft
	{
		float stepLength = 0.05f;
		float precision = 1e-4f;
		bool useCursor = false;
	};

	bool DrawIntersectionDraft(IntersectionDraft& draft, bool canIntersect)
	{
		if (!ImGui::BeginPopupModal(GUI::IntersectionDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return false;
		}

		if (ImGui::IsWindowAppearing())
		{
			draft = IntersectionDraft{};
		}

		constexpr float formWidth = 300.f;

		if (GUI::BeginPropertyTable("##IntersectionDraft", formWidth))
		{
			GUI::PropertyRow("Step length", draft.stepLength, 0.002f, 0.005f, 0.25f, "%.3f");
			GUI::PropertyRow("Precision", draft.precision, 0.00005f, 1e-5f, 1e-2f, "%.5f");
			GUI::PropertyRow("Use 3D cursor", draft.useCursor);
			GUI::EndPropertyTable();
		}

		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + formWidth);
		ImGui::TextDisabled("Select one surface (self-intersection) or two, then Create.");
		ImGui::PopTextWrapPos();

		ImGui::Separator();

		bool create = false;

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		ImGui::BeginDisabled(!canIntersect);
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button("Create"))
		{
			create = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();
		ImGui::EndDisabled();

		ImGui::EndPopup();
		return create;
	}
}

void GUI::DrawIntersectionDialog(UiState& state, Ref<Scene> scene)
{
	(void)state;

	static IntersectionDraft draft;

	if (!DrawIntersectionDraft(draft, GUI::CanCreateIntersection(scene)))
	{
		return;
	}

	Geometry::IntersectionSettings settings;
	settings.stepLength = std::clamp(draft.stepLength, 0.005f, 0.25f);
	settings.precision = std::clamp(draft.precision, 1e-5f, 1e-2f);
	settings.useCursor = draft.useCursor;

	GUI::CreateIntersection(scene, settings);
}
