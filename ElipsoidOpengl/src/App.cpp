#include "App.h"
#include <string>
#include <stdexcept>
#include "Shader.h"

App::App()
    : window{640 + 200, 480, "Elipsoid"}, running{true}
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
#ifdef _DEBUG
        ImGui::ShowDemoWindow();
#endif 

        HandleInput();
        Update();

        DisplayParameters();
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
    raycaster.RayCast(&window);
}

void App::DisplayParameters()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoDocking;

    ImGui::SetNextWindowPos(ImVec2(window.GetWidth() - 200, 0.f));
    ImGui::SetNextWindowSize(ImVec2(200, window.GetHeight()));

    ImGui::Begin("Main Menu", nullptr, window_flags);

    raycaster.RenderMenu();

    ImGui::End();
}

void App::Render()
{
    raycaster.RenderResult();
}