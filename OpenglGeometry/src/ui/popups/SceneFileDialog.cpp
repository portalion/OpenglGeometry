#include "SceneFileDialog.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "serialization/SceneSerialization.h"

namespace
{
	namespace fs = std::filesystem;

	std::string SceneRelativePath(const fs::path& absolutePath)
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
			name = "untitled.json";
		}
		return name;
	}

	bool IsJsonFile(const fs::path& file)
	{
		std::string ext = file.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return ext == ".json";
	}

	void DrawPathField(std::string& path)
	{
		ImGui::TextUnformatted("Path");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(360.f);
		ImGui::InputText("##Path", &path);
	}

	bool DrawFileBrowser(std::string& path)
	{
		static fs::path browseDir;
		static bool initialised = false;

		std::error_code ec;

		if (!initialised)
		{
			const fs::path parent = fs::path(path).parent_path();
			browseDir = (!parent.empty() && fs::is_directory(parent, ec))
				? fs::absolute(parent, ec)
				: fs::current_path(ec);
			initialised = true;
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

		if (ImGui::BeginChild("##FileBrowser", ImVec2(360.f, 170.f), ImGuiChildFlags_Border))
		{
			if (browseDir.has_parent_path() && browseDir.parent_path() != browseDir)
			{
				if (ImGui::Selectable(".."))
				{
					browseDir = browseDir.parent_path();
					path = SceneRelativePath(browseDir / FileNameOf(path));
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
				else if (entry.is_regular_file(entryEc) && IsJsonFile(entry.path()))
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
					path = SceneRelativePath(browseDir / FileNameOf(path));
				}
			}

			const fs::path currentFile = fs::absolute(fs::path(path), ec);

			for (const fs::path& file : files)
			{
				const bool selected = fs::equivalent(file, currentFile, ec);

				if (ImGui::Selectable(file.filename().string().c_str(), selected,
					ImGuiSelectableFlags_AllowDoubleClick))
				{
					path = SceneRelativePath(file);

					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						confirmed = true;
					}
				}
			}

			if (directories.empty() && files.empty())
			{
				ImGui::TextDisabled("no .json files here");
			}
		}
		ImGui::EndChild();

		return confirmed;
	}

	bool DrawFileDialogBody(const char* title, std::string& path, const char* confirmLabel)
	{
		if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return false;
		}

		DrawPathField(path);
		const bool browserConfirmed = DrawFileBrowser(path);

		ImGui::TextDisabled("Gregory patches and intersection curves cannot be represented.");

		ImGui::Separator();

		bool confirmed = browserConfirmed;
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(confirmLabel))
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
}

void GUI::DrawSaveSceneDialog(UiState& state)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(SaveSceneDialogTitle, path, "Save"))
	{
		state.statusMessage = "saved to " + path + " (sandbox - no writer wired up)";
	}
}

void GUI::DrawOpenSceneDialog(UiState& state)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(OpenSceneDialogTitle, path, "Open"))
	{
		state.statusMessage = "would open " + path + " (sandbox - no loader wired up)";
	}
}

void GUI::DrawSaveSceneDialog(UiState& state, Ref<Scene> scene)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(SaveSceneDialogTitle, path, "Save"))
	{
		state.statusMessage = Serialization::SaveScene(*scene, path).message;
	}
}

void GUI::DrawOpenSceneDialog(UiState& state, Ref<Scene> scene)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(OpenSceneDialogTitle, path, "Open"))
	{
		state.statusMessage = Serialization::LoadScene(*scene, path).message;
	}
}
