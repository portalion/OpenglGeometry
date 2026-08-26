#pragma once
#include "core/Base.h"
#include "model/UiState.h"

class Scene;
class Dockspace;

namespace GUI
{
	void DrawCursorPanel(UiState& state);
	void DrawCursorPanel(Ref<Scene> scene, UiState& state, const Dockspace& dockspace);
}
