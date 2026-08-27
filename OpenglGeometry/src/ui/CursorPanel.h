#pragma once
#include <functional>
#include "Algebra.h"
#include "core/Base.h"
#include "model/UiState.h"

class Scene;
class Dockspace;

namespace GUI
{
	struct CursorPanelCallbacks
	{
		std::function<void(PivotMode pivot,
			Algebra::Vector4 moveBy,
			Algebra::Vector4 rotateByDegrees,
			Algebra::Vector4 scaleBy)> applySelectionTransform;
	};

	void DrawCursorPanel(UiState& state);
	void DrawCursorPanel(Ref<Scene> scene, UiState& state, const Dockspace& dockspace,
		const CursorPanelCallbacks* callbacks = nullptr);
}
