#include "FileDialog.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <unordered_map>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

namespace
{
	namespace fs = std::filesystem;

	std::string RelativePath(const fs::path& absolutePath)
	{
		std::error_code ec;
		const fs::path relative = fs::relative(absolutePath, fs::current_path(ec), ec);
		const std::string text = relative.generic_string();

		if (ec || text.empty() || text.rfind("..", 0) == 0)
		{
			return absolutePath.generic_string();
		}

		return text;
	}

	fs::path FileNameOf(const std::string& path)
	{
		fs::path name = fs::path(path).filename();
		if (name.empty())
		{
			name = "untitled";
		}
		return name;
	}

	bool MatchesExtension(const fs::path& file, const std::vector<std::string>& extensions)
	{
		if (extensions.empty())
		{
			return true;
		}

		std::string ext = file.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
	}

	void DrawPathField(std::string& path)
	{
		ImGui::TextUnformatted("Path");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(360.f);
		ImGui::InputText("##Path", &path);
	}
}

bool GUI::DrawFileBrowser(const char* id, std::string& path, const std::vector<std::string>& extensions, ImVec2 size)
{
	static std::unordered_map<std::string, fs::path> browseDirs;
	const bool initialised = browseDirs.contains(id);
	fs::path& browseDir = browseDirs[id];

	std::error_code ec;

	if (!initialised)
	{
		const fs::path parent = fs::path(path).parent_path();
		browseDir = (!parent.empty() && fs::is_directory(parent, ec))
			? fs::absolute(parent, ec)
			: fs::current_path(ec);
	}

	if (!fs::is_directory(browseDir, ec))
	{
		browseDir = fs::current_path(ec);
	}

	{
		const fs::path typedParent = fs::path(path).parent_path();
		if (!typedParent.empty())
		{
			const fs::path abs = fs::absolute(typedParent, ec);
			if (!ec && fs::is_directory(abs, ec) && !fs::equivalent(abs, browseDir, ec))
			{
				browseDir = abs;
			}
		}
	}

	ImGui::TextDisabled("%s", browseDir.generic_string().c_str());

	bool confirmed = false;

	if (ImGui::BeginChild(id, size, ImGuiChildFlags_Border))
	{
		if (browseDir.has_parent_path() && browseDir.parent_path() != browseDir)
		{
			if (ImGui::Selectable(".."))
			{
				browseDir = browseDir.parent_path();
				path = RelativePath(browseDir / FileNameOf(path));
			}
		}

		std::vector<fs::path> directories;
		std::vector<fs::path> files;

		for (fs::directory_iterator it(browseDir, fs::directory_options::skip_permission_denied, ec);
			it != fs::directory_iterator(); it.increment(ec))
		{
			if (ec)
			{
				break;
			}

			const fs::directory_entry& entry = *it;
			std::error_code entryEc;

			if (entry.is_directory(entryEc))
			{
				directories.push_back(entry.path());
			}
			else if (entry.is_regular_file(entryEc) && MatchesExtension(entry.path(), extensions))
			{
				files.push_back(entry.path());
			}
		}

		const auto byFileName = [](const fs::path& a, const fs::path& b)
		{
			return a.filename().string() < b.filename().string();
		};
		std::sort(directories.begin(), directories.end(), byFileName);
		std::sort(files.begin(), files.end(), byFileName);

		for (const fs::path& directory : directories)
		{
			const std::string label = "[ " + directory.filename().string() + " ]";
			if (ImGui::Selectable(label.c_str()))
			{
				browseDir = directory;
				path = RelativePath(browseDir / FileNameOf(path));
			}
		}

		const fs::path currentFile = fs::absolute(fs::path(path), ec);

		for (const fs::path& file : files)
		{
			const bool selected = fs::equivalent(file, currentFile, ec);

			if (ImGui::Selectable(file.filename().string().c_str(), selected,
				ImGuiSelectableFlags_AllowDoubleClick))
			{
				path = RelativePath(file);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					confirmed = true;
				}
			}
		}

		if (directories.empty() && files.empty())
		{
			ImGui::TextDisabled("no matching files here");
		}
	}
	ImGui::EndChild();

	return confirmed;
}

bool GUI::DrawFileDialog(const char* title, std::string& path, const FileDialogOptions& options)
{
	if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return false;
	}

	DrawPathField(path);
	const bool browserConfirmed = GUI::DrawFileBrowser(title, path, options.extensions, ImVec2(360.f, 170.f));

	if (options.note != nullptr)
	{
		ImGui::TextDisabled("%s", options.note);
	}

	ImGui::Separator();

	bool confirmed = browserConfirmed;
	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button(options.confirmLabel))
	{
		confirmed = true;
	}

	if (confirmed)
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
	return confirmed;
}
