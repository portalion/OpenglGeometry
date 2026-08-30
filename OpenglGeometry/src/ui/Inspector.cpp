#include "Inspector.h"

#include <algorithm>
#include <array>
#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "Utils.h"
#include "Widgets.h"

namespace
{
	using namespace GUI;

	ObjectRow* FindMutable(UiState& state, uint32_t id)
	{
		for (ObjectRow& row : state.objects)
		{
			if (row.id == id)
			{
				return &row;
			}
		}

		return nullptr;
	}

	bool SectionHeader(const char* label)
	{
		return ImGui::CollapsingHeader(label,
			ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
	}

	void DrawHeaderBlock(ObjectRow& row)
	{
		ImGui::SetNextItemWidth(-160.f);
		ImGui::InputText("##Name", &row.name);

		ImGui::SameLine();
		ImGui::BeginDisabled();
		ImGui::SmallButton(ToDisplayString(row.type));
		ImGui::EndDisabled();

		ImGui::SameLine();
		TextDisabledRightAligned(("id " + std::to_string(row.id)).c_str());
	}

	void DrawTransformRows(TransformValues& transform, bool rotationAndScale)
	{
		if (!BeginPropertyTable("##Transform"))
		{
			return;
		}

		PropertyRow("Position", transform.position);
		if (rotationAndScale)
		{
			PropertyRow("Rotation", transform.rotationEuler);
			PropertyRow("Scale", transform.scale);
		}
		EndPropertyTable();
	}

	void DrawTorusRows(TorusValues& torus)
	{
		if (!BeginPropertyTable("##Torus"))
		{
			return;
		}

		PropertyRow("Large radius", torus.largeRadius, 0.05f);
		PropertyRow("Tube radius", torus.tubeRadius, 0.05f);
		PropertyRow("Samples U", torus.samplesU, 3u, 64u);
		PropertyRow("Samples V", torus.samplesV, 3u, 64u);
		EndPropertyTable();
	}

	void DrawTransformSection(TransformValues& transform)
	{
		if (SectionHeader("TRANSFORM"))
		{
			DrawTransformRows(transform, true);
		}
	}

	void DrawTorusSection(TorusValues& torus)
	{
		if (SectionHeader("TORUS"))
		{
			DrawTorusRows(torus);
			ImGui::TextDisabled("3 - 64 samples per ring");
		}
	}

	void DrawCurveSection(UiState& state, CurveValues& curve)
	{
		if (!SectionHeader("CURVE"))
		{
			return;
		}

		static constexpr std::array<const char*, 2> basisOptions = { "de Boor", "Bernstein" };
		int basisIndex = curve.bernsteinBasis ? 1 : 0;

		ImGui::TextUnformatted("Basis");
		ImGui::SameLine();
		if (SegmentedControl("##Basis", basisIndex, basisOptions))
		{
			curve.bernsteinBasis = (basisIndex == 1);
		}

		ImGui::Spacing();
		ImGui::Text("Control points (%zu)", curve.controlPoints.size());

		int moveUp = -1;
		int moveDown = -1;
		int removeIndex = -1;

		for (int i = 0; i < static_cast<int>(curve.controlPoints.size()); i++)
		{
			ObjectRow& point = curve.controlPoints[static_cast<std::size_t>(i)];

			ImGui::PushID(static_cast<int>(point.id));

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(point.name.c_str());

			ImGui::SameLine(180.f);
			ImGui::TextDisabled("id %u", point.id);

			ImGui::SameLine(260.f);
			ImGui::BeginDisabled(i == 0);
			if (ImGui::SmallButton("^"))
			{
				moveUp = i;
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::BeginDisabled(i + 1 == static_cast<int>(curve.controlPoints.size()));
			if (ImGui::SmallButton("v"))
			{
				moveDown = i;
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.30f, 0.30f, 1.f));
			if (ImGui::SmallButton("x"))
			{
				removeIndex = i;
			}
			ImGui::PopStyleColor();

			ImGui::PopID();
		}

		if (moveUp > 0)
		{
			std::swap(curve.controlPoints[static_cast<std::size_t>(moveUp)],
				curve.controlPoints[static_cast<std::size_t>(moveUp - 1)]);
		}

		if (moveDown >= 0 && moveDown + 1 < static_cast<int>(curve.controlPoints.size()))
		{
			std::swap(curve.controlPoints[static_cast<std::size_t>(moveDown)],
				curve.controlPoints[static_cast<std::size_t>(moveDown + 1)]);
		}

		if (removeIndex >= 0)
		{
			curve.controlPoints.erase(curve.controlPoints.begin() + removeIndex);
		}

		const std::size_t selectedCount = state.SelectedCount();

		ImGui::BeginDisabled(selectedCount == 0);
		if (ImGui::Button("Add selected points"))
		{
			for (const ObjectRow* selected : state.Selected())
			{
				if (selected->type != ObjectType::Point)
				{
					continue;
				}

				const bool alreadyIn = std::ranges::any_of(curve.controlPoints,
					[selected](const ObjectRow& point) { return point.id == selected->id; });

				if (!alreadyIn)
				{
					curve.controlPoints.push_back(*selected);
				}
			}
		}
		ImGui::EndDisabled();

		ImGui::Checkbox("Show polygon", &curve.showPolygon);
		ImGui::SameLine();
		ImGui::Checkbox("Show virtual points", &curve.showVirtualPoints);
	}

	void DrawSurfaceSection(SurfaceValues& surface)
	{
		if (!SectionHeader("SURFACE"))
		{
			return;
		}

		if (BeginPropertyTable("##Surface"))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Size");
			ImGui::TableSetColumnIndex(1);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%u x %u", surface.sizeU, surface.sizeV);

			PropertyRow("Tessellation U", surface.samplesU, 2u, 64u);
			PropertyRow("Tessellation V", surface.samplesV, 2u, 64u);
			PropertyRow("Show control net", surface.showControlNet);
			EndPropertyTable();
		}

		if (ImGui::Button("Select control points"))
		{
			surface.selectPointsRequested = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%u points", surface.controlPointCount);
	}

	void DrawSelectionSummary(UiState& state)
	{
		std::array<int, ObjectTypeCount> counts{};

		for (const ObjectRow* row : state.Selected())
		{
			counts[static_cast<std::size_t>(row->type)]++;
		}

		bool first = true;

		for (ObjectType type : AllObjectTypes)
		{
			const int count = counts[static_cast<std::size_t>(type)];

			if (count == 0)
			{
				continue;
			}

			if (!first)
			{
				ImGui::SameLine();
			}

			first = false;

			ImGui::BeginDisabled();
			ImGui::SmallButton((std::to_string(count) + " x " + ToFormatString(type)).c_str());
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("centre  %.3f, %.3f, %.3f",
			state.cursor.selectionCentre.x, state.cursor.selectionCentre.y, state.cursor.selectionCentre.z);
	}

	void DrawPerObjectEntry(ObjectRow& row)
	{
		ImGui::PushID(static_cast<int>(row.id));

		const std::string label =
			row.name + "      " + ToFormatString(row.type) + "  #" + std::to_string(row.id) + "###entry";

		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent();

			if (BeginPropertyTable("##Name"))
			{
				Detail::BeginPropertyValue("Name");
				ImGui::InputText("##NameValue", &row.name);
				EndPropertyTable();
			}

			if (row.transform)
			{
				DrawTransformRows(*row.transform, row.type != ObjectType::Point);
			}

			if (row.torus)
			{
				ImGui::Spacing();
				DrawTorusRows(*row.torus);
			}

			ImGui::Unindent();
			ImGui::Spacing();
		}

		ImGui::PopID();
	}

	void DrawPerObjectSection(UiState& state)
	{
		ImGui::SeparatorText("Per object");

		for (ObjectRow& row : state.objects)
		{
			if (row.selected)
			{
				DrawPerObjectEntry(row);
			}
		}
	}
}

void GUI::DrawInspector(UiState& state)
{
	ImGui::Begin(InspectorWindow);

	const std::size_t selectedCount = state.SelectedCount();

	if (selectedCount == 0)
	{
		ImGui::TextDisabled("No objects selected.");
		ImGui::End();
		return;
	}

	if (selectedCount == 1)
	{
		const uint32_t id = state.Selected().front()->id;
		ObjectRow* row = FindMutable(state, id);

		ImGui::PushID(static_cast<int>(row->id));

		DrawHeaderBlock(*row);
		ImGui::Separator();

		if (state.transform) DrawTransformSection(*state.transform);
		if (state.torus)     DrawTorusSection(*state.torus);
		if (state.curve)     DrawCurveSection(state, *state.curve);
		if (state.surface)   DrawSurfaceSection(*state.surface);

		ImGui::PopID();
	}
	else
	{
		ImGui::Text("%zu objects selected", selectedCount);
		DrawSelectionSummary(state);
		ImGui::Separator();

		if (state.surface)
		{
			DrawSurfaceSection(*state.surface);
			ImGui::Separator();
		}

		DrawPerObjectSection(state);
	}

	ImGui::End();
}
