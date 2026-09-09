#pragma once
#include "core/Base.h"
#include "model/UiState.h"

class Scene;

namespace GUI
{
	inline constexpr const char* ParameterSpaceWindow = "Parameter space##ParameterSpace";

	void DrawParameterSpace(Ref<Scene> scene, UiState& state);
}
