#pragma once
#include <string>
#include <vector>
#include <imgui/imgui.h>

namespace GUI
{
	struct FileDialogOptions
	{
		const char* confirmLabel = "Open";
		// Lower-case, with the dot (".json"); empty lists every file
		std::vector<std::string> extensions;
		const char* note = nullptr;
	};

	// Folder browser that writes the clicked file into path; returns true on double-click
	bool DrawFileBrowser(const char* id, std::string& path, const std::vector<std::string>& extensions, ImVec2 size);

	// Draws the modal opened with ImGui::OpenPopup(title); returns true when the user confirms
	bool DrawFileDialog(const char* title, std::string& path, const FileDialogOptions& options);
}
