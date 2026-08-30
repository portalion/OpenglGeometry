#pragma once
#include "core/Base.h"
#include "scene/Scene.h"
#include "ui/model/UiState.h"

namespace GUI
{
	inline constexpr const char* SaveSceneDialogTitle = "Save scene##SaveSceneDialog";
	inline constexpr const char* OpenSceneDialogTitle = "Open scene##OpenSceneDialog";

	void DrawSaveSceneDialog(UiState& state);
	void DrawOpenSceneDialog(UiState& state);

	void DrawSaveSceneDialog(UiState& state, Ref<Scene> scene);
	void DrawOpenSceneDialog(UiState& state, Ref<Scene> scene);
}
