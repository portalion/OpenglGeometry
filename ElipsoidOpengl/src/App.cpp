#include "App.h"
#include <string>
#include <stdexcept>
#include "Shader.h"

App::App()
    : window{640, 480, "Elipsoid"}, running{true}
{
    InitImgui(window.GetWindowPointer());
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
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif 

        HandleInput();
        Update();
        Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.ProcessFrame();
    }
}

void App::HandleInput()
{
}

void App::Update()
{
}

void App::Render()
{
    raycaster.RenderResult();
}