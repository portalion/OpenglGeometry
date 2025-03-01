#include "App.h"
#include <string>
#include <stdexcept>

App::App()
    : window{640, 480, "Elipsoid"}, running{true}
{
    running &= InitGLEW();
    running &= InitImgui(window.GetWindowPointer());
    if (!running)
    {
        throw std::runtime_error("cannot initialize app");
    }
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
}