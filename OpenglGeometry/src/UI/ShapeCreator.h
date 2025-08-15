#pragma once
#include "scene/Scene.h"
#include <imgui/imgui.h>
#include "archetypes/Archetypes.h"

namespace GUI
{
	inline void DisplayCreationButtons(Ref<Scene> scene)
	{
		ImGui::Begin("Creation Menu##Creation menu");

		if (ImGui::Button("Create Torus##Creation menu"))
		{
			Archetypes::CreateTorus(scene.get());
		}

		ImGui::End();
	}
}
