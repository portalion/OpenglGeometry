#pragma once
#include <cstdint>
#include <span>
#include <imgui/imgui.h>
#include "Algebra.h"
#include "Style.h"

namespace GUI
{
	inline constexpr float PropertyLabelWidth = 110.f;

	inline bool BeginPropertyTable(const char* id)
	{
		constexpr ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit;

		if (!ImGui::BeginTable(id, 2, flags))
		{
			return false;
		}

		ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, PropertyLabelWidth);
		ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

		return true;
	}

	inline void EndPropertyTable()
	{
		ImGui::EndTable();
	}

	namespace Detail
	{
		inline void BeginPropertyValue(const char* label)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
		}
	}

	inline bool PropertyRow(const char* label, Algebra::Vector4& value, float speed = 0.1f)
	{
		Detail::BeginPropertyValue(label);
		ImGui::PushID(label);
		const bool changed = ImGui::DragFloat3("##Value", &value.x, speed);
		ImGui::PopID();
		return changed;
	}

	inline bool PropertyRow(const char* label, float& value, float speed = 0.1f)
	{
		Detail::BeginPropertyValue(label);
		ImGui::PushID(label);
		const bool changed = ImGui::DragFloat("##Value", &value, speed);
		ImGui::PopID();
		return changed;
	}

	inline bool PropertyRow(const char* label, uint32_t& value, uint32_t min, uint32_t max)
	{
		Detail::BeginPropertyValue(label);
		ImGui::PushID(label);
		const bool changed = ImGui::DragScalar("##Value", ImGuiDataType_U32, &value, 1.0f, &min, &max);
		ImGui::PopID();
		return changed;
	}

	inline bool PropertyRow(const char* label, bool& value)
	{
		Detail::BeginPropertyValue(label);
		ImGui::PushID(label);
		const bool changed = ImGui::Checkbox("##Value", &value);
		ImGui::PopID();
		return changed;
	}

	inline bool SegmentedControl(const char* id, int& index, std::span<const char* const> options)
	{
		bool changed = false;

		ImGui::PushID(id);

		for (std::size_t i = 0; i < options.size(); i++)
		{
			if (i > 0)
			{
				ImGui::SameLine(0.f, Style::SegmentedButtonSpacing);
			}

			const bool active = index == static_cast<int>(i);

			if (active)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			}

			if (ImGui::Button(options[i]))
			{
				if (!active)
				{
					index = static_cast<int>(i);
					changed = true;
				}
			}

			if (active)
			{
				ImGui::PopStyleColor();
			}
		}

		ImGui::PopID();

		return changed;
	}
}
