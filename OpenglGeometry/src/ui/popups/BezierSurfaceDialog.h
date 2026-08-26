#pragma once
#include "core/Base.h"
#include "scene/Scene.h"
#include "ui/model/UiState.h"

namespace GUI
{
	inline constexpr const char* BezierSurfaceDialogTitle = "Create Bezier surface##BezierSurfaceDialog";

	void DrawBezierSurfaceDialog(UiState& state);
	void DrawBezierSurfaceDialog(UiState& state, Ref<Scene> scene);
}
