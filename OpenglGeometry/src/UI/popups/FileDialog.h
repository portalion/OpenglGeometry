#pragma once
#include <string>
#include <vector>

namespace GUI
{
	struct FileDialogOptions
	{
		const char* confirmLabel = "Open";
		// Lower-case, with the dot (".json"); empty lists every file
		std::vector<std::string> extensions;
		const char* note = nullptr;
	};

	// Draws the modal opened with ImGui::OpenPopup(title); returns true when the user confirms
	bool DrawFileDialog(const char* title, std::string& path, const FileDialogOptions& options);
}
