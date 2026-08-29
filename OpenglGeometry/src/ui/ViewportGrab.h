#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"
#include "Algebra.h"

class Scene;
class Dockspace;

namespace GUI
{
	struct ViewportGrabState
	{
		bool active = false;
		bool hasLastHit = false;
		Algebra::Vector4 planePoint{ 0.f, 0.f, 0.f, 1.f };
		Algebra::Vector4 lastHit{ 0.f, 0.f, 0.f, 1.f };
		Algebra::Vector4 totalDelta{ 0.f, 0.f, 0.f, 0.f };
	};

	bool HandleViewportGrab(Ref<Scene> scene, const Dockspace& dockspace, ViewportGrabState& state);
}
