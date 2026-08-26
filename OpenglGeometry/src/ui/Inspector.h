#pragma once
#include <functional>
#include "Algebra.h"
#include "model/UiState.h"

namespace GUI
{
	struct InspectorCallbacks
	{
		std::function<void(PivotMode pivot,
			Algebra::Vector4 moveBy,
			Algebra::Vector4 rotateByDegrees,
			Algebra::Vector4 scaleBy)> applySelectionTransform;
	};

	void DrawInspector(UiState& state, const InspectorCallbacks* callbacks = nullptr);
}
