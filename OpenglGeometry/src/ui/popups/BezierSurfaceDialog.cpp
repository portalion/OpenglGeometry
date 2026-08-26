#include "BezierSurfaceDialog.h"

#include <array>
#include <imgui/imgui.h>

#include "ui/Widgets.h"
#include "archetypes/Archetypes.h"

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

	bool DrawBezierSurfaceDraft(BezierSurfaceDraft& draft)
	{
		if (!ImGui::BeginPopupModal(GUI::BezierSurfaceDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return false;
		}

		if (ImGui::IsWindowAppearing())
		{
			draft = BezierSurfaceDraft{};
		}

		static constexpr std::array<const char*, 2> continuityOptions = { "C0", "C2" };

		ImGui::TextUnformatted("Continuity");
		ImGui::SameLine();
		GUI::SegmentedControl("##Continuity", draft.continuityIndex, continuityOptions);

		if (GUI::BeginPropertyTable("##BezierSurfaceDraft"))
		{
			GUI::PropertyRow("Patches U", draft.patchesU, 1u, 32u);
			GUI::PropertyRow("Patches V", draft.patchesV, 1u, 32u);
			GUI::PropertyRow("Size U", draft.sizeU, 0.1f);
			GUI::PropertyRow("Size V", draft.sizeV, 0.1f);
			GUI::PropertyRow("Wrap into a cylinder", draft.cylinder);
			GUI::PropertyRow("Samples U", draft.samplesU, 1u, 64u);
			GUI::PropertyRow("Samples V", draft.samplesV, 1u, 64u);
			GUI::EndPropertyTable();
		}

		ImGui::TextDisabled("%u control points", ControlPointCount(draft));
		ImGui::TextDisabled("created at the 3D cursor");

		ImGui::Separator();

		bool create = false;

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Create"))
		{
			create = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
		return create;
	}
}

void GUI::DrawBezierSurfaceDialog(UiState& state)
{
	static BezierSurfaceDraft draft;

	if (DrawBezierSurfaceDraft(draft))
	{
		const ObjectType type = draft.continuityIndex == 1 ? ObjectType::BezierSurfaceC2 : ObjectType::BezierSurfaceC0;
		state.AppendObject(type, "Surface");
	}
}

void GUI::DrawBezierSurfaceDialog(UiState& state, Ref<Scene> scene)
{
	(void)state;

	static BezierSurfaceDraft draft;

	if (!DrawBezierSurfaceDraft(draft))
	{
		return;
	}

	Scene* raw = scene.get();

	Archetypes::BezierSurfaceCreationParameters params;
	params.isCylinder = draft.cylinder;
	params.sizeX = draft.sizeU;
	params.sizeY = draft.sizeV;
	params.numberOfXPatches = draft.patchesU;
	params.numberOfYPatches = draft.patchesV;
	params.startingPosition = Archetypes::GetCursorPosition(raw);

	Entity surface = Archetypes::CreateBezierSurface(raw, params);

	auto& generation = surface.GetComponent<BezierSurfaceGenerationComponent>();
	generation.samplesU = static_cast<int>(draft.samplesU);
	generation.samplesV = static_cast<int>(draft.samplesV);
}
