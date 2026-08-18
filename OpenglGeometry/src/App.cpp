#include "App.h"
#include <string>
#include <stdexcept>
#include <iostream>
#include "core/Globals.h"
#include <scene/BaseScene.h>

App::App()
    : m_Window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}
{
    InitImgui(m_Window.GetWindowPointer());
    ImGui::StyleColorsDark();
    m_Window.SetAppPointerData(this);

	m_CurrentScene = CreateRef<BaseScene>();
	m_SystemPipeline = CreateUnique<SystemPipeline>(m_CurrentScene, m_Viewport);
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

App& App::GetInstance()
{
    static App instance;
    return instance;
}


void App::Run()
{
    while (!m_Window.ShouldClose())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (m_SystemPipeline)
		{
			m_SystemPipeline->Update();
		}

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_Window.ProcessFrame();
    }
}
