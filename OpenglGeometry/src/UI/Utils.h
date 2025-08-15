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
}
