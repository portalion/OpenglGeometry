#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"

class Scene;
class Dockspace;
struct CursorState;

namespace GUI
{
	struct CursorPlacementState
	{
		bool   tracking = false;
		ImVec2 pressPos{ 0.f, 0.f };
	};

	void HandleCursorPlacement(Ref<Scene> scene, const Dockspace& dockspace,
		const CursorState& cursorState, CursorPlacementState& state);
}
