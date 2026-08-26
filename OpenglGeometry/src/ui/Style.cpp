#include "Style.h"

#include <imgui/imgui.h>

void GUI::ApplyStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.FramePadding = ImVec2(8.f, 4.f);
	style.ItemSpacing = ImVec2(8.f, 6.f);
	style.WindowPadding = ImVec2(10.f, 10.f);
	style.FrameRounding = 2.f;
	style.WindowRounding = 4.f;
	style.GrabRounding = 2.f;
	style.TabRounding = 3.f;
	style.ScrollbarRounding = 4.f;
	style.PopupRounding = 4.f;

	ImVec4* colors = style.Colors;
	const ImVec4 sectionHeader(0.106f, 0.122f, 0.149f, 1.f);
	colors[ImGuiCol_Header] = sectionHeader;
	colors[ImGuiCol_HeaderHovered] = sectionHeader;
	colors[ImGuiCol_HeaderActive] = sectionHeader;

	colors[ImGuiCol_TextDisabled] = ImVec4(0.514f, 0.549f, 0.600f, 1.f);
}
