#include "CAMGUISystem.h"
#include <filesystem>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <algorithm>
#include <utils/Initialization.h>
#include <ui/layouts/CAMLayout.h>
#include <ui/popups/FileDialog.h>
#include <serialization/PathFileReader.h>
#include <core/Log.h>
#include <archetypes/Archetypes.h>
namespace
{
	inline constexpr const char* SelectPathDialogTitle = "Select path##SelectPathDialog";
}

CAMGUISystem::CAMGUISystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport)
{
}

void CAMGUISystem::DrawMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Exit"))
		{
			glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Reset layout"))
		{
			GUI::Layout::CAM(m_Dockspace);
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void CAMGUISystem::DrawPathPanel()
{
	if (!ImGui::Begin(GUI::CAMPathPanelWindow))
	{
		ImGui::End();
		return;
	}

	std::error_code ec;
	const bool hasFile = std::filesystem::is_regular_file(m_PathFile, ec);

	if (ImGui::Button("Browse..."))
	{
		m_BrowsePath = m_PathFile;
		ImGui::OpenPopup(SelectPathDialogTitle);
	}
	ImGui::SameLine();
	if (hasFile)
	{
		ImGui::TextUnformatted(std::filesystem::path(m_PathFile).stem().string().c_str());
	}
	else
	{
		ImGui::TextDisabled("none");
	}

	ImGui::BeginDisabled(!hasFile);
	if (ImGui::Button("Load", ImVec2(-FLT_MIN, 0.f)))
	{
		LoadPath(m_PathFile);
	}
	ImGui::EndDisabled();

	if (GUI::DrawFileDialog(SelectPathDialogTitle, m_BrowsePath, { "Select" }))
	{
		m_PathFile = m_BrowsePath;
	}

	DrawLoadedPaths();

	ImGui::End();
}

void CAMGUISystem::DrawLoadedPaths()
{
	ImGui::SeparatorText("Loaded paths");

	auto paths = m_Scene->GetAllEntitiesWith<PathComponent>();
	if (paths.begin() == paths.end())
	{
		ImGui::TextDisabled("No paths loaded");
		return;
	}

	for (Entity entity : paths)
	{
		auto& tool = entity.GetComponent<PathComponent>().m_Tool;
		auto& name = entity.GetComponent<NameComponent>().name;
		const auto id = entity.GetComponent<IdComponent>().id;
		ImGui::PushID(static_cast<int>(id));

		const std::string header = name + "###PathHeader";
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool visible = !entity.HasComponent<IsInvisibleTag>();
			if (ImGui::Checkbox("Visible", &visible))
			{
				if (visible)
				{
					entity.RemoveTag<IsInvisibleTag>();
				}
				else
				{
					entity.AddTag<IsInvisibleTag>();
				}
			}

			ImGui::InputText("Name", &name);

			int toolType = tool.m_IsFlat ? 1 : 0;
			if (ImGui::Combo("Tool", &toolType, "Ball\0Flat\0"))
			{
				tool.m_IsFlat = toolType == 1;
			}

			if (ImGui::InputFloat("Diameter", &tool.m_Diameter, 1.f, 5.f, "%.0f mm"))
			{
				tool.m_Diameter = std::max(tool.m_Diameter, 1.f);
			}
		}

		ImGui::PopID();
	}
}

void CAMGUISystem::LoadPath(const std::string& file)
{
	PathFileReader reader;
	auto path = reader.ReadFile(file);

	auto pathEntity = m_Scene->CreateEntity();
	
	Archetypes::AddPathToEntity(pathEntity, path);
	pathEntity.GetComponent<NameComponent>().name = std::filesystem::path(file).filename().string();
}

void CAMGUISystem::Process()
{
	DrawMenuBar();

	if (!m_Dockspace.Created())
	{
		GUI::Layout::CAM(m_Dockspace);
	}

	m_Dockspace.ClaimSize();

	ViewportData sceneRect;
	if (m_Dockspace.TryGetCentralNodeRect(sceneRect))
	{
		m_Viewport.Change(sceneRect);
	}

	DrawPathPanel();
}
