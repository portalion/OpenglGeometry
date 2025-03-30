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
    currentInputMode->HandleInput(sceneRenderables);
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
        axis.DisplayMenu();
    }

    this->CreateShape();
    if (ImGui::CollapsingHeader("Selected item parameters", ImGuiTreeNodeFlags_Leaf))
    {
        for (auto& renderable : sceneRenderables)
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
                bool isSelected = selectedShapes.count(shapePtr) > 0;

                if (ImGui::Selectable(shapePtr->GetName().c_str(), isSelected))
                {
                    if (ctrlPressed)
                    {
                        if (isSelected)
                            selectedShapes.erase(shapePtr);
                        else
                            selectedShapes.insert(shapePtr);
                    }
                    else
                    {
                        selectedShapes.clear();
                        selectedShapes.insert(shapePtr);
                    }
                }
            }
        }

        ImGui::EndChild();
        std::string buffer;
        ImGui::InputText("Shape name", &buffer);
        
        if (ImGui::Button("Create shape"))
        {
            auto newShape = std::make_shared<Torus>();
            newShape->InitName();
            newShape->Move(axis.GetPosition());
            sceneRenderables.push_back(newShape);
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(selectedShapes.size() == 0);
        if (ImGui::Button("Remove shape"))
        {
            sceneRenderables.erase(
                std::remove_if(sceneRenderables.begin(), sceneRenderables.end(),
                    [&](const std::shared_ptr<RenderableOnScene>& shape) {
                        bool shouldRemove = selectedShapes.find(shape) != selectedShapes.end();
                        return shouldRemove; // Assuming 'id' is an identifier for shapes
                    }),
                sceneRenderables.end()
            );
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Rename shape"))
        {
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