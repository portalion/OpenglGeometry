#include "BezierSurfaceDialog.h"

#include <array>
#include <utility>
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

	std::pair<uint32_t, uint32_t> ControlPointGrid(const BezierSurfaceDraft& draft)
	{
		const uint32_t pointsU = draft.cylinder ? draft.patchesU * 3 : draft.patchesU * 3 + 1;
		const uint32_t pointsV = draft.patchesV * 3 + 1;
		return { pointsU, pointsV };
	}

	void DrawControlNetPreview(const BezierSurfaceDraft& draft)
	{
		const auto [pointsU, pointsV] = ControlPointGrid(draft);

		ImGui::BeginChild("##SurfacePreview", ImVec2(184.f, 158.f), ImGuiChildFlags_Border,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();
		const float pad = 14.f;
		const float footer = ImGui::GetTextLineHeightWithSpacing() + 4.f;
		const ImVec2 area(size.x - pad * 2.f, size.y - pad * 2.f - footer);

		const bool wrap = draft.cylinder;
		const uint32_t columns = wrap ? pointsU + 1 : pointsU;

		auto point = [&](uint32_t i, uint32_t j)
		{
			const float fx = columns > 1 ? static_cast<float>(i) / static_cast<float>(columns - 1) : 0.5f;
			const float fy = pointsV > 1 ? static_cast<float>(j) / static_cast<float>(pointsV - 1) : 0.5f;
			return ImVec2(origin.x + pad + fx * area.x, origin.y + pad + fy * area.y);
		};

		const ImU32 netColor = ImGui::GetColorU32(ImGuiCol_Separator);
		const ImU32 dotColor = ImGui::GetColorU32(ImVec4(1.f, 0.29f, 0.08f, 1.f));

		for (uint32_t j = 0; j < pointsV; j++)
		{
			for (uint32_t i = 0; i + 1 < columns; i++)
			{
				drawList->AddLine(point(i, j), point(i + 1, j), netColor, 1.f);
			}
		}
		for (uint32_t i = 0; i < columns; i++)
		{
			for (uint32_t j = 0; j + 1 < pointsV; j++)
			{
				drawList->AddLine(point(i, j), point(i, j + 1), netColor, 1.f);
			}
		}
		for (uint32_t i = 0; i < columns; i++)
		{
			const bool seam = wrap && i == columns - 1;
			for (uint32_t j = 0; j < pointsV; j++)
			{
				drawList->AddCircleFilled(point(i, j), 2.f, seam ? netColor : dotColor);
			}
		}

		ImGui::SetCursorPos(ImVec2(pad, size.y - ImGui::GetTextLineHeight() - pad * 0.5f));
		ImGui::TextDisabled("%u control points", pointsU * pointsV);

		ImGui::EndChild();
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

		constexpr float formWidth = 300.f;

		ImGui::BeginGroup();
		if (GUI::BeginPropertyTable("##BezierSurfaceDraft", formWidth))
		{
			GUI::PropertyRowSegmented("Continuity", draft.continuityIndex, continuityOptions);
			GUI::PropertyRowUV("Patches", draft.patchesU, draft.patchesV, 1u, 32u);
			GUI::PropertyRowUV("Size", draft.sizeU, draft.sizeV, 0.1f);
			GUI::PropertyCheckboxRow("Wrap into a cylinder", draft.cylinder);
			GUI::PropertyRowUV("Samples", draft.samplesU, draft.samplesV, 1u, 64u);
			GUI::EndPropertyTable();
		}
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + formWidth);
		ImGui::TextDisabled("Samples default to 4 and can be changed later in the inspector.");
		ImGui::PopTextWrapPos();
		ImGui::EndGroup();

		ImGui::SameLine();
		DrawControlNetPreview(draft);

		ImGui::Separator();

		ImGui::AlignTextToFramePadding();
		ImGui::TextDisabled("created at the 3D cursor");
		ImGui::SameLine();

		const ImGuiStyle& style = ImGui::GetStyle();
		const float cancelWidth = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
		const float createWidth = ImGui::CalcTextSize("Create").x + style.FramePadding.x * 2.f;
		const float buttonsWidth = cancelWidth + createWidth + style.ItemSpacing.x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonsWidth);

		bool create = false;

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button("Create"))
		{
			create = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

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
