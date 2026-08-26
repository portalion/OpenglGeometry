#include "ParameterSpace.h"

#include <array>
#include <cstddef>
#include <vector>
#include <imgui/imgui.h>

#include "Widgets.h"

namespace
{
	struct ParameterSpaceDraft
	{
		int seedMode = 0;  // 0 = first intersection, 1 = manual
		float stepLength = 0.05f;
		float tolerance = 0.001f;
		int trimSide = 0;  // 0 = keep left, 1 = keep right
	};

	ParameterSpaceDraft& Draft()
	{
		static ParameterSpaceDraft draft;
		return draft;
	}

	const std::vector<ImVec2>& FixturePolyline()
	{
		static const std::vector<ImVec2> points =
		{
			{0.08f, 0.62f}, {0.22f, 0.40f}, {0.38f, 0.48f}, {0.52f, 0.22f},
			{0.68f, 0.30f}, {0.80f, 0.16f}, {0.92f, 0.24f},
		};

		return points;
	}

	void DrawUnitSquare(const char* id, const char* label, const std::vector<ImVec2>& polyline, int trimSide)
	{
		ImGui::BeginGroup();
		ImGui::TextUnformatted(label);

		ImGui::PushID(id);
		ImGui::BeginChild("##Square", ImVec2(220.f, 220.f), ImGuiChildFlags_Border);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();

		const auto toPixel = [&](ImVec2 uv)
		{
			return ImVec2(origin.x + uv.x * size.x, origin.y + (1.f - uv.y) * size.y);
		};

		drawList->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y), IM_COL32(11, 12, 15, 255));

		for (int i = 1; i < 4; i++)
		{
			const float t = static_cast<float>(i) / 4.f;
			drawList->AddLine(toPixel({ t, 0.f }), toPixel({ t, 1.f }), IM_COL32(30, 34, 40, 255));
			drawList->AddLine(toPixel({ 0.f, t }), toPixel({ 1.f, t }), IM_COL32(30, 34, 40, 255));
		}

		const float splitU = 0.5f;
		const ImVec2 shadeMin = trimSide == 0 ? toPixel({ splitU, 1.f }) : toPixel({ 0.f, 1.f });
		const ImVec2 shadeMax = trimSide == 0 ? toPixel({ 1.f, 0.f }) : toPixel({ splitU, 0.f });
		drawList->AddRectFilled(shadeMin, shadeMax, IM_COL32(255, 74, 20, 30));

		for (std::size_t i = 1; i < polyline.size(); i++)
		{
			drawList->AddLine(toPixel(polyline[i - 1]), toPixel(polyline[i]), IM_COL32(255, 74, 20, 230), 1.6f);
		}

		for (const ImVec2& point : polyline)
		{
			drawList->AddCircleFilled(toPixel(point), 3.f, IM_COL32(255, 74, 20, 255));
		}

		ImGui::EndChild();
		ImGui::PopID();
		ImGui::EndGroup();
	}
}

void GUI::DrawParameterSpace(UiState&)
{
	ImGui::Begin(ParameterSpaceWindow);

	ParameterSpaceDraft& draft = Draft();
	const std::vector<ImVec2>& polyline = FixturePolyline();

	DrawUnitSquare("##SquareA", "Surface A", polyline, draft.trimSide);
	ImGui::SameLine();
	DrawUnitSquare("##SquareB", "Surface B", polyline, draft.trimSide);

	ImGui::SameLine();
	ImGui::BeginGroup();

	static constexpr std::array<const char*, 2> seedOptions = { "First intersection", "Manual" };
	ImGui::TextUnformatted("Seed");
	SegmentedControl("##Seed", draft.seedMode, seedOptions);

	if (BeginPropertyTable("##ParamSpace"))
	{
		PropertyRow("Step length", draft.stepLength, 0.005f);
		PropertyRow("Tolerance", draft.tolerance, 0.0005f);
		EndPropertyTable();
	}

	static constexpr std::array<const char*, 2> trimOptions = { "Keep left", "Keep right" };
	ImGui::TextUnformatted("Trim side");
	SegmentedControl("##TrimSide", draft.trimSide, trimOptions);

	ImGui::Spacing();

	ImGui::BeginDisabled(true);
	ImGui::Button("Trace");
	ImGui::SameLine();
	ImGui::Button("Convert to interpolating C2");
	ImGui::EndDisabled();

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("needs the tracer - phase 9");
	}

	ImGui::EndGroup();

	ImGui::End();
}
