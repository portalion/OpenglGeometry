#include "App.h"
#include <string>
#include <stdexcept>
#include <iostream>
#include "core/Globals.h"
#include <scene/BaseScene.h>

App::App()
    : window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}, 
    running{true}
{
    InitImgui(window.GetWindowPointer());
    ImGui::StyleColorsDark();
    window.SetAppPointerData(this);

	currentScene = CreateRef<BaseScene>();
	systemPipeline = CreateUnique<SystemPipeline>(currentScene);
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}


void App::Run()
{
    while (running && !window.ShouldClose())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (systemPipeline)
		{
			systemPipeline->Update();
		}

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.ProcessFrame();
    }
}
