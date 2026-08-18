#pragma once
#include <imgui/imgui_internal.h>
#include <core/Viewport.h>

class Dockspace
{
private:
	ImGuiID m_Id;
public:
	Dockspace(const char* id = "MainDockspace");

	bool Created();
	ImGuiID StartCreation(ImVec2 size);
	void FinishCreation();
	void ClaimSize();
	void FixViewportSize(Viewport& viewport);
};