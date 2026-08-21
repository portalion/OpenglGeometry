#pragma once
#include <imgui/imgui.h>
#include <string>
#include <functional>
#include "scene/Entity.h"
#include "scene/Components.h"

namespace GUI
{
	inline constexpr const char* ShapeListWindow = "Shape List##Shape List";
	inline constexpr const char* InspectorWindow = "Selected Shapes Properties##Selected Shapes Properties";

	struct UICallbacks
	{
		std::function<void()> resetLayout;
		std::function<void()> renameSelected;
	};

	inline std::string GenerateLabel(Entity entity, std::string labelWithoutId)
	{
		std::string result = labelWithoutId;

		const auto& id = entity.GetHandleId();

		result += "##" + id;
		return result;
	}

	inline bool DragUInt(const char* label, uint32_t* value, float speed = 1.0f, uint32_t min = 0, uint32_t max = 0)
	{
		return ImGui::DragScalar(label, ImGuiDataType_U32, value, speed, &min, &max);
	}
	inline void TextDisabledRightAligned(const char* text)
	{
		const float width = ImGui::CalcTextSize(text).x;
		const float x = ImGui::GetContentRegionMax().x - width;

		if (x > ImGui::GetCursorPosX())
		{
			ImGui::SetCursorPosX(x);
		}

		ImGui::TextDisabled("%s", text);
	}
}
