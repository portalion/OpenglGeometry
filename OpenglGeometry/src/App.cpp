#include "App.h"
#include <string>
#include <stdexcept>
#include "engine/Shader.h"
#include <iostream>
#include "core/Globals.h"
#include <engine/Renderer.h>
#include <core/InfiniteGrid.h>

App::App()
    : window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}, 
    running{true},
	camera{ Globals::startingCameraPosition, 1.f}
{
    InitImgui(window.GetWindowPointer());
    window.SetAppPointerData(this);
	viewMatrix = Algebra::Matrix4::Identity();

    HandleResize();
    defaultShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);

	torus = new Torus();
    torus->InitName();
}

App::~App()
{
    delete torus;

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
    if (ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        return;
    }

    camera.HandleInput();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        auto mousePos = ImGui::GetMousePos();
        draggingPoint = GetMousePoint(mousePos.x, mousePos.y).Normalize();
        return;
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        auto mousePos = ImGui::GetMousePos();
        Algebra::Vector4 q = GetMousePoint(mousePos.x, mousePos.y).Normalize();
        if (q == draggingPoint)
        {
            return;
        }
        float theta = acosf(draggingPoint * q);
        auto w = q.Cross(draggingPoint).Normalize();
        auto tempMat = Algebra::Matrix4(0, 0, 0, 0);
        tempMat[1][0] = w.z;
        tempMat[0][1] = -w.z;
        tempMat[0][2] = -w.y;
        tempMat[2][0] = w.y;
        tempMat[2][1] = w.x;
        tempMat[1][2] = -w.x;
        auto rotation = Algebra::Matrix4::Identity() + sinf(theta) * tempMat + ((1.f - cosf(theta)) * tempMat * tempMat);
        draggingPoint = q;
    }
}

void App::HandleResize()
{
	float newWidth = static_cast<float>(window.GetWidth() - Globals::rightInterfaceWidth);
	float newHeight = static_cast<float>(window.GetHeight());
	float aspect = newWidth / newHeight;
    projectionMatrix = Algebra::Matrix4::Projection(aspect, 0.1f, 10000.0f, 3.14f / 2.f);
}

void App::Update()
{
    torus->Update();
}

void App::DisplayParameters()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoDocking;

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.GetWidth() - Globals::rightInterfaceWidth), 0.f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(Globals::rightInterfaceWidth), static_cast<float>(window.GetHeight())));

    ImGui::Begin("Main Menu", nullptr, window_flags);
    if (ImGui::CollapsingHeader("Main Menu", ImGuiTreeNodeFlags_Leaf))
    {
		ImGui::Checkbox("Show grid", &showGrid);
    }

    if (ImGui::CollapsingHeader("Selected item parameters", ImGuiTreeNodeFlags_Leaf))
    {
        torus->DisplayMenu();
    }
    ImGui::End();
}

Algebra::Vector4 App::GetMousePoint(float x, float y)
{
    float screenWidth = static_cast<float>(window.GetWidth());
    float screenHeight = static_cast<float>(window.GetHeight());
    float scale = fminf(screenHeight, screenWidth) - 1.f;

    x = (2.f * x - screenWidth + 1.f) / scale;
    y = (2.f * y - screenHeight + 1.f) / -scale;

    float z = 0;
    float d = x * x + y * y;
    if (d <= 1.f / 2.f)
    {
        z = sqrtf(1 - d);
    }
    else
    {
        z = 1.f / 2.f / sqrtf(d);
    }

    return Algebra::Vector4(x, y, z, 0);
}

void App::Render()
{
    if (showGrid)
    {
	    grid.Render(camera.GetViewMatrix(), projectionMatrix, camera.GetPosition());
    }

	defaultShader->Bind();
    defaultShader->SetUniformMat4f("u_viewMatrix", camera.GetViewMatrix());
    defaultShader->SetUniformMat4f("u_projectionMatrix", projectionMatrix);
    torus->Render();
	defaultShader->UnBind();
}