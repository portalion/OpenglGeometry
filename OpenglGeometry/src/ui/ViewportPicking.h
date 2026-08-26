#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"

class Scene;
class Dockspace;

namespace GUI
{
	struct ViewportPickingState
	{
		bool   dragging = false;
		ImVec2 dragStart{ 0.f, 0.f };
	};

	void HandleViewportPicking(Ref<Scene> scene, const Dockspace& dockspace, ViewportPickingState& state);
}
