#include "UiSandbox.h"

#include <imgui/imgui_internal.h>

#include "core/Globals.h"
#include "core/Window.h"
#include "ui/CursorPanel.h"
#include "ui/Inspector.h"
#include "ui/MenuItems.h"
#include "ui/ParameterSpace.h"
#include "ui/Style.h"
#include "ui/Toolbar.h"
#include "ui/Utils.h"
#include "ui/core/DockSpace.h"
#include "ui/model/Fixture.h"
#include "utils/Initialization.h"

namespace
{
	constexpr const char* ObjectsWindow = "Objects##Sandbox";

	void BuildLayout(Dockspace& dockspace)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImGuiID dockspaceId = dockspace.StartCreation(viewport->WorkSize);

		ImGuiID rightId = 0;
		ImGuiID centralId = 0;
		ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.32f, &rightId, &centralId);

		ImGuiID objectsId = 0;
		ImGuiID restId = 0;
		ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Down, 0.72f, &restId, &objectsId);

		ImGuiID cursorId = 0;
		ImGuiID inspectorId = 0;
		ImGui::DockBuilderSplitNode(restId, ImGuiDir_Down, 0.35f, &cursorId, &inspectorId);

		ImGui::DockBuilderDockWindow(ObjectsWindow, objectsId);
		ImGui::DockBuilderDockWindow(GUI::InspectorWindow, inspectorId);
		ImGui::DockBuilderDockWindow(GUI::CursorPanelWindow, cursorId);
		ImGui::DockBuilderDockWindow(GUI::ParameterSpaceWindow, centralId);

		dockspace.FinishCreation();
	}

	void DrawMenuBar(UiState& state, bool& showImGuiDemo, bool& showParameterSpace, bool& resetLayout, bool& quit)
	{
		if (!ImGui::BeginMainMenuBar())
		{
			return;
		}

		if (ImGui::BeginMenu("File"))
		{
			GUI::DrawFileMenuItems(state);
			ImGui::Separator();
			quit |= ImGui::MenuItem("Exit");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			GUI::DrawCreateMenuItems(state);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			GUI::DrawViewDisplayItems(state, showParameterSpace);
			ImGui::Separator();
			ImGui::MenuItem("Dear ImGui demo", nullptr, &showImGuiDemo);
			resetLayout |= ImGui::MenuItem("Reset layout");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				ImGui::OpenPopup(GUI::AboutDialogTitle);
			}
			ImGui::EndMenu();
		}

		ImGui::TextDisabled("   UI sandbox - fixture data, nothing is wired up");

		ImGui::EndMainMenuBar();
	}

	void DrawStatusBar(const UiState& state)
	{
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		const float height = ImGui::GetFrameHeight() + GUI::Style::StatusBarExtraHeight;

		if (ImGui::BeginViewportSideBar("##SandboxStatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, height, flags))
		{
			ImGui::Text("%.0f fps", ImGui::GetIO().Framerate);
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("%zu objects, %zu selected", state.objects.size(), state.SelectedCount());
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("pivot: %s", ToDisplayString(state.pivot));
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("cursor %.3f, %.3f, %.3f",
				state.cursor.world.x, state.cursor.world.y, state.cursor.world.z);

			if (!state.statusMessage.empty())
			{
				ImGui::SameLine();
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
				ImGui::SameLine();
				ImGui::TextDisabled("%s", state.statusMessage.c_str());
			}
		}
		ImGui::End();
	}

	void DrawObjects(UiState& state)
	{
		ImGui::Begin(ObjectsWindow);

		ImGui::TextDisabled("%zu objects, %zu selected", state.objects.size(), state.SelectedCount());
		ImGui::Separator();

		if (ImGui::BeginChild("##SandboxObjectList"))
		{
			for (ObjectRow& row : state.objects)
			{
				ImGui::PushID(static_cast<int>(row.id));

				const char* label = row.name.empty() ? "(unnamed)" : row.name.c_str();

				if (ImGui::Selectable(label, row.selected))
				{
					row.selected = !row.selected;
				}

				ImGui::SameLine();
				ImGui::TextDisabled("%s  #%u", ToDisplayString(row.type), row.id);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}
}

int UiSandbox::Run()
{
	Window window{ Globals::startingSceneWidth + Globals::rightInterfaceWidth,
		Globals::startingSceneHeight, "Geometry - UI sandbox" };
	window.SetAppPointerData(nullptr);

	if (!InitImgui(window.GetWindowPointer()))
	{
		return -1;
	}

	ImGui::StyleColorsDark();
	GUI::ApplyStyle();

	ImGui::GetIO().IniFilename = "imgui-sandbox.ini";

	UiState state = GUI::MakeFixture();
	Dockspace dockspace{ "SandboxDockspace" };

	bool showImGuiDemo = false;
	bool showParameterSpace = true;
	bool quit = false;

	while (!quit && !window.ShouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		bool resetLayout = false;
		DrawMenuBar(state, showImGuiDemo, showParameterSpace, resetLayout, quit);
		GUI::DrawToolbar(state);
		DrawStatusBar(state);

		GUI::DrawAllDialogs(state);
		GUI::HandleFileShortcuts();

		if (resetLayout || !dockspace.Created())
		{
			BuildLayout(dockspace);
		}

		dockspace.ClaimSize();

		DrawObjects(state);
		GUI::DrawInspector(state);
		GUI::DrawCursorPanel(state);

		if (showParameterSpace)
		{
			GUI::DrawParameterSpace(state);
		}

		if (showImGuiDemo)
		{
			ImGui::ShowDemoWindow(&showImGuiDemo);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.ProcessFrame();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}
