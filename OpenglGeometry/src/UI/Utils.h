#pragma once
#include <string>
#include "scene/Entity.h"
#include "scene/Components.h"

namespace GUI
{
	inline std::string GenerateLabel(Entity entity, std::string labelWithoutId)
	{
		std::string result = labelWithoutId;

		const auto& id = entity.GetID();

		result += "##" + id;
		return result;
	}

	inline bool DragUInt(const char* label, uint32_t* value, float speed = 1.0f, uint32_t min = 0, uint32_t max = 0)
	{
		return ImGui::DragScalar(label, ImGuiDataType_U32, value, speed, &min, &max);
	}
}
