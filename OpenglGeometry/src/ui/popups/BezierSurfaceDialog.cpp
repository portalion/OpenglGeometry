#include "BezierSurfaceDialog.h"

#include <array>
#include <imgui/imgui.h>

#include "ui/Widgets.h"

namespace
{
	struct BezierSurfaceDraft
	{
		int continuityIndex = 0; // 0 = C0, 1 = C2
		uint32_t patchesU = 3;
		uint32_t patchesV = 2;
		float sizeU = 4.f;
		float sizeV = 3.f;
		bool cylinder = false;
		uint32_t samplesU = 4;
		uint32_t samplesV = 4;
	};

	uint32_t ControlPointCount(const BezierSurfaceDraft& draft)
	{
		const uint32_t pointsU = draft.cylinder ? draft.patchesU * 3 : draft.patchesU * 3 + 1;
		const uint32_t pointsV = draft.patchesV * 3 + 1;
		return pointsU * pointsV;
	}
}

void GUI::DrawBezierSurfaceDialog(UiState& state)
{
	if (!ImGui::BeginPopupModal(BezierSurfaceDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	static BezierSurfaceDraft draft;

	if (ImGui::IsWindowAppearing())
	{
		draft = BezierSurfaceDraft{};
	}

	static constexpr std::array<const char*, 2> continuityOptions = { "C0", "C2" };

	ImGui::TextUnformatted("Continuity");
	ImGui::SameLine();
	SegmentedControl("##Continuity", draft.continuityIndex, continuityOptions);

	if (BeginPropertyTable("##BezierSurfaceDraft"))
	{
		PropertyRow("Patches U", draft.patchesU, 1u, 32u);
		PropertyRow("Patches V", draft.patchesV, 1u, 32u);
		PropertyRow("Size U", draft.sizeU, 0.1f);
		PropertyRow("Size V", draft.sizeV, 0.1f);
		PropertyRow("Wrap into a cylinder", draft.cylinder);
		PropertyRow("Samples U", draft.samplesU, 1u, 64u);
		PropertyRow("Samples V", draft.samplesV, 1u, 64u);
		EndPropertyTable();
	}

	ImGui::TextDisabled("%u control points", ControlPointCount(draft));
	ImGui::TextDisabled("created at the 3D cursor");

	ImGui::Separator();

	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Create"))
	{
		const ObjectType type = draft.continuityIndex == 1 ? ObjectType::BezierSurfaceC2 : ObjectType::BezierSurfaceC0;
		state.AppendObject(type, "Surface");
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
