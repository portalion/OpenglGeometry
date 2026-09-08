#pragma once
#include "core/Base.h"
#include "scene/Scene.h"
#include "ui/model/UiState.h"

namespace GUI
{
	inline constexpr const char* IntersectionDialogTitle = "Intersection curve##IntersectionDialog";

	void DrawIntersectionDialog(UiState& state, Ref<Scene> scene);
}
