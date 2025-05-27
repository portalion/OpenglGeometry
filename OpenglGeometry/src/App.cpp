#include "App.h"
#include <string>
#include <stdexcept>
#include "engine/Shader.h"
#include <iostream>
#include "core/Globals.h"
#include <engine/Renderer.h>
#include <core/InfiniteGrid.h>
#include <objects/lines/Polyline.h>
#include <objects/lines/BezierCurve.h>
#include <objects/lines/BezierCurveC2.h>
#include <objects/lines/InterpolatedBezierCurve.h>
#include <fstream>

Algebra::Matrix4 App::projectionMatrix;
Camera App::camera = Camera(Globals::startingCameraPosition, 1.f);
Window* App::windowStatic = nullptr;
App::App()
    : window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}, 
    running{true},
    shapeList{ &axis, &selectedShapes }
{
    InitImgui(window.GetWindowPointer());
    window.SetAppPointerData(this);
	windowStatic = &window;
    HandleResize();
    defaultShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);

    currentInputMode = InputMode::CreateInputMode(InputModeEnum::Default, &window, &camera, &axis);
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
#ifdef _DEBUG
        ImGui::ShowDemoWindow();
#endif 

        HandleInput();
        DisplayParameters();

        Update();
        Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.ProcessFrame();
    }
}

void App::HandleInput()
{
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        GetClickedPoint();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        selectedShapes.Clear();
    }
	//TODO: Add iterator to selectedShapes
    auto shapes = selectedShapes.GetSelectedWithType<RenderableOnScene>();
    currentInputMode->HandleInput({shapes.begin(), shapes.end()});
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
    shapeList.Update();
    axis.Update();
    middleSelectionPoint.Update();

    if(auto middlePoint = selectedShapes.GetAveragePosition())
        middleSelectionPoint.SetPosition(*middlePoint);

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
        static int currentModeIndex = 0;
        const auto& modes = InputMode::GetModeList();

        std::vector<const char*> modeNames;
        std::vector<InputModeEnum> modeEnums;

        for (const auto& [enumVal, modeName] : modes) {
            modeEnums.push_back(enumVal);
            modeNames.push_back(modeName.c_str());
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Tab))
        {
            currentModeIndex = (currentModeIndex + 1) % static_cast<int>(InputModeEnum::Size);
            currentInputMode = InputMode::CreateInputMode(modeEnums[currentModeIndex], &window, &camera, &axis);
        }

        if (ImGui::Combo("Input Mode", &currentModeIndex, modeNames.data(), static_cast<int>(modeNames.size()))) {
            currentInputMode = InputMode::CreateInputMode(modeEnums[currentModeIndex], &window, &camera, &axis);
        }
        axis.DisplayMenu();
    }
    //TODO: Add iterator to selectedShapes
    
    shapeList.DisplayUI();
    
    //TODO: Change to json manager
    if (ImGui::Button("Save scene"))
    {
		std::ofstream file("scene.json");
		file << shapeList.SerializeList().dump(4);
    }
	ImGui::SameLine();
    if (ImGui::Button("Load scene"))
    {
        json j;
        std::ifstream file("scene.json");
        file >> j;
        shapeList.Deserialize(j);
    }

    if (ImGui::CollapsingHeader("Selected items parameters", ImGuiTreeNodeFlags_Leaf))
    {
        for (auto& renderable : selectedShapes.GetSelectedWithType<RenderableOnScene>())
        {
            renderable->DisplayMenu();
        }
    }
    ImGui::End();
}

Algebra::Vector4 App::ScreenToNDC(float x, float y)
{
    float ndcX = (2.0f * x) / (window.GetWidth() - Globals::rightInterfaceWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * y) / window.GetHeight();
    
    return Algebra::Vector4(ndcX, ndcY, 0.f, 1.f);
}

void App::GetClickedPoint()
{
    auto screenPos = ImGui::GetMousePos();
    auto ndcPos = ScreenToNDC(screenPos.x, screenPos.y);

    if (std::abs(ndcPos.x) > 1.f || std::abs(ndcPos.y) > 1.f)
    {
        return;
    }

   
    bool isCtrlPressed = ImGui::GetIO().KeyCtrl;
    
    const auto& shape = shapeList.GetPointByPosition(projectionMatrix * camera.GetViewMatrix(), ndcPos);
    if (!shape)
    {
        return;
    }

    if (isCtrlPressed)
    {
	    selectedShapes.ToggleShape(shape);
    }
    else
    {
        selectedShapes.Clear();
        selectedShapes.AddShape(shape);
    }
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
    defaultShader->SetUniformVec4f("u_color", Globals::defaultPointsColor);
    
    shapeList.Render();

    defaultShader->SetUniformMat4f("u_modelMatrix", Algebra::Matrix4::Translation(axis.GetPosition()));
    defaultShader->SetUniformMat4f("u_viewMatrix", camera.GetRotationMatrix() * camera.GetTranslationMatrix());
    axis.Render();
    
    defaultShader->SetUniformMat4f("u_viewMatrix", camera.GetViewMatrix());
    defaultShader->SetUniformVec4f("u_color", Globals::defaultMiddlePointColor);

    if (!selectedShapes.IsEmpty())
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", middleSelectionPoint.GetModelMatrix());
        middleSelectionPoint.Render();
    }
  
    defaultShader->UnBind();
}