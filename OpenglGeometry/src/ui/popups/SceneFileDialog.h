#pragma once
#include "ui/model/UiState.h"

namespace GUI
{
	inline constexpr const char* SaveSceneDialogTitle = "Save scene##SaveSceneDialog";
	inline constexpr const char* OpenSceneDialogTitle = "Open scene##OpenSceneDialog";

	void DrawSaveSceneDialog(UiState& state);
	void DrawOpenSceneDialog(UiState& state);
}
