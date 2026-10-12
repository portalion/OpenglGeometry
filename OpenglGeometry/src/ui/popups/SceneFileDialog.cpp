#include "SceneFileDialog.h"

#include <string>

#include "FileDialog.h"
#include "serialization/SceneSerialization.h"

namespace
{
	const char* SceneNote = "Gregory patches and intersection curves cannot be represented.";

	bool DrawFileDialogBody(const char* title, std::string& path, const char* confirmLabel)
	{
		return GUI::DrawFileDialog(title, path, { confirmLabel, { ".json" }, SceneNote });
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
		Serialization::SaveScene(*scene, path);
	}
}

void GUI::DrawOpenSceneDialog(UiState& state, Ref<Scene> scene)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(OpenSceneDialogTitle, path, "Open"))
	{
		Serialization::LoadScene(*scene, path);
	}
}
