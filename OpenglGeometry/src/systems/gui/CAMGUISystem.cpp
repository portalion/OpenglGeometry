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

CAMGUISystem::CAMGUISystem(Ref<Scene> scene, Ref<CAMSettings> settings, Viewport& viewport)
	:m_Scene(scene), m_Settings(settings), m_Viewport(viewport)
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

	DrawMaterialSection();
	DrawSimulationSection();

	ImGui::SeparatorText("Load paths");
	std::error_code ec;
	const bool hasFile = std::any_of(m_PathFiles.begin(), m_PathFiles.end(),
		[&](const std::string& file) { return std::filesystem::is_regular_file(file, ec); });

	if (ImGui::Button("Browse..."))
	{
		m_BrowsePaths = m_PathFiles;
		ImGui::OpenPopup(SelectPathDialogTitle);
	}
	ImGui::SameLine();
	if (m_PathFiles.size() == 1)
	{
		ImGui::TextUnformatted(std::filesystem::path(m_PathFiles.front()).stem().string().c_str());
	}
	else if (!m_PathFiles.empty())
	{
		ImGui::Text("%zu files", m_PathFiles.size());
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			for (const auto& file : m_PathFiles)
			{
				ImGui::TextUnformatted(std::filesystem::path(file).filename().string().c_str());
			}
			ImGui::EndTooltip();
		}
	}
	else
	{
		ImGui::TextDisabled("none");
	}

	ImGui::BeginDisabled(!hasFile);
	if (ImGui::Button("Load", ImVec2(-FLT_MIN, 0.f)))
	{
		for (const auto& file : m_PathFiles)
		{
			if (std::filesystem::is_regular_file(file, ec))
			{
				LoadPath(file);
			}
		}
	}
	ImGui::EndDisabled();

	if (GUI::DrawMultiFileDialog(SelectPathDialogTitle, m_BrowsePaths, { "Select" }))
	{
		m_PathFiles = m_BrowsePaths;
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

void CAMGUISystem::DrawMaterialSection()
{
	ImGui::SeparatorText("Material");

	auto& material = m_Settings->m_Material;
	ImGui::BeginDisabled(m_Settings->m_Simulation.m_State != SimulationState::Stopped);

	if (ImGui::DragFloat3("Size [mm]", material.m_Size.data(), 1.f, 1.f, 1000.f, "%.1f"))
	{
		for (float& size : material.m_Size)
		{
			size = std::max(size, 1.f);
		}
		material.m_MaxDepth = std::min(material.m_MaxDepth, material.m_Size[2]);
	}

	if (ImGui::DragInt2("Divisions X/Y", material.m_Resolution.data(), 10.f, 2, 4000))
	{
		for (int& resolution : material.m_Resolution)
		{
			resolution = std::clamp(resolution, 2, 4000);
		}
	}
	ImGui::TextDisabled("Cell: %.3f x %.3f mm",
		material.m_Size[0] / material.m_Resolution[0],
		material.m_Size[1] / material.m_Resolution[1]);

	ImGui::SliderFloat("Max depth [mm]", &material.m_MaxDepth, 0.f, material.m_Size[2], "%.1f");
	ImGui::EndDisabled();
}

void CAMGUISystem::DrawSimulationSection()
{
	ImGui::SeparatorText("Simulation");

	auto& simulation = m_Settings->m_Simulation;
	const bool running = simulation.m_State == SimulationState::Running;
	const bool finished = simulation.m_State == SimulationState::Finished;

	const float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

	ImGui::BeginDisabled(finished);
	if (ImGui::Button(running ? "Pause" : "Start", ImVec2(buttonWidth, 0.f)))
	{
		simulation.m_State = running ? SimulationState::Paused : SimulationState::Running;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(simulation.m_State == SimulationState::Stopped);
	if (ImGui::Button("Reset", ImVec2(buttonWidth, 0.f)))
	{
		simulation.m_State = SimulationState::Stopped;
		simulation.m_Progress = 0.f;
		simulation.m_ResetRequested = true;
	}
	ImGui::EndDisabled();

	ImGui::BeginDisabled(finished);
	if (ImGui::Button("Finish instantly", ImVec2(-FLT_MIN, 0.f)))
	{
		simulation.m_FinishRequested = true;
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("Run the whole simulation without preview");
	}

	ImGui::SliderFloat("Speed [mm/s]", &simulation.m_Speed, 1.f, 1000.f, "%.0f", ImGuiSliderFlags_Logarithmic);

	ImGui::ProgressBar(simulation.m_Progress, ImVec2(-FLT_MIN, 0.f));
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
