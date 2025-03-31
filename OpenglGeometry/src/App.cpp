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

    sceneRenderables.push_back(std::make_shared<Torus>());
    sceneRenderables.push_back(std::make_shared<Point>());
    for (auto& renderable : sceneRenderables)
    {
        renderable->InitName();
    }
    
    currentInputMode = InputMode::CreateInputMode(InputModeEnum::Default, &window, &camera);
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

std::shared_ptr<RenderableOnScene> App::CreateNewShape(AvailableShapes shape)
{
    switch (shape)
    {
    case AvailableShapes::Point:
        return std::make_shared<Point>();
    case AvailableShapes::Torus:
        return std::make_shared<Torus>();
    }
    throw std::runtime_error("Invalid shape");
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
    currentInputMode->HandleInput(selectedRenderables);
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
    for (auto& renderable : sceneRenderables)
    {
        renderable->Update();
    }
    axis.Update();
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

        if (ImGui::Combo("Input Mode", &currentModeIndex, modeNames.data(), static_cast<int>(modeNames.size()))) {
            currentInputMode = InputMode::CreateInputMode(modeEnums[currentModeIndex], &window, &camera);
        }
        axis.DisplayMenu();
    }

    this->CreateShape();
    if (ImGui::CollapsingHeader("Selected items parameters", ImGuiTreeNodeFlags_Leaf))
    {
        for (auto& renderable : selectedRenderables)
        {
            renderable->DisplayMenu();
        }
    }
    ImGui::End();
}

void App::CreateShape()
{
    if(ImGui::CollapsingHeader("Shape List", ImGuiTreeNodeFlags_Leaf))
    {
        ImGui::BeginChild("ShapeList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (sceneRenderables.empty())
        {
            ImGui::Text("No shapes available.");
        }
        else
        {
            bool ctrlPressed = ImGui::GetIO().KeyCtrl;

            for (const auto& renderable : sceneRenderables)
            {
                std::shared_ptr<RenderableOnScene> shapePtr = renderable;
                bool isSelected = selectedRenderables.count(shapePtr) > 0;

                if (ImGui::Selectable(shapePtr->GetName().c_str(), isSelected))
                {
                    if (ctrlPressed)
                    {
                        if (isSelected)
                            selectedRenderables.erase(shapePtr);
                        else
                            selectedRenderables.insert(shapePtr);
                    }
                    else
                    {
                        selectedRenderables.clear();
                        selectedRenderables.insert(shapePtr);
                    }
                }
            }
        }
        ImGui::EndChild();
        const char* items[] = { "Point", "Torus" };
        static int item = 0;
        
        if (ImGui::Button("Create shape"))
        {
            AvailableShapes shapeType = static_cast<AvailableShapes>(item);
            auto newShape = CreateNewShape(shapeType);
            newShape->InitName();
            newShape->Move(axis.GetPosition());
            sceneRenderables.push_back(newShape);
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Globals::rightInterfaceWidth / 3.f);
        ImGui::Combo("##shape", &item, items, IM_ARRAYSIZE(items)); 
        ImGui::SameLine();

        ImGui::BeginDisabled(selectedRenderables.size() == 0);
        if (ImGui::Button("Remove shapes"))
        {
            sceneRenderables.erase(
                std::remove_if(sceneRenderables.begin(), sceneRenderables.end(),
                    [&](const std::shared_ptr<RenderableOnScene>& shape) {
                        bool shouldRemove = selectedRenderables.find(shape) != selectedRenderables.end();
                        return shouldRemove; 
                    }),
                sceneRenderables.end()
            );
        }
        ImGui::EndDisabled();
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
    for (auto& renderable : sceneRenderables)
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", renderable->GetModelMatrix());
        renderable->Render();
    }
    defaultShader->SetUniformMat4f("u_modelMatrix", axis.GetModelMatrix());
    axis.Render();
	defaultShader->UnBind();
}