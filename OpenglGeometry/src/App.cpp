#include "App.h"
#include <string>
#include <stdexcept>
#include "engine/Shader.h"
#include <iostream>
#include "core/Globals.h"
#include <engine/Renderer.h>
#include <core/InfiniteGrid.h>
#include <objects/Polyline.h>

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

    for (auto& renderable : sceneRenderables)
    {
        renderable->InitName();
    }
    
    currentInputMode = InputMode::CreateInputMode(InputModeEnum::Default, &window, &camera, &axis);
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
    case AvailableShapes::Polyline:
        return std::make_shared<Polyline>(selectedPoints);
    }
    throw std::runtime_error("Invalid shape");
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
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        GetClickedPoint();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        selectedRenderables.clear();
    }

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
    middleSelectionPoint.Update();
    if (!selectedRenderables.empty())
    {
        Algebra::Vector4 middlePoint;
        for (auto& selected : selectedRenderables)
        {
            middlePoint += selected->GetPosition();
        }
        middlePoint = middlePoint / selectedRenderables.size();
        middleSelectionPoint.SetPosition(middlePoint);
    }
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

                if (ImGui::Selectable(shapePtr->GenerateLabelWithId(shapePtr->GetName()).c_str(), isSelected))
                {
                    if (ctrlPressed)
                    {
                        if (isSelected)
                        {
                            selectedRenderables.erase(shapePtr);
                            if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                            {
                                std::erase(selectedPoints, point);
                            }
                        }
                        else
                        {
                            selectedRenderables.insert(shapePtr);
                            if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                            {
                                selectedPoints.push_back(point);
                            }
                        }
                    }
                    else
                    {
                        selectedRenderables.clear();
                        selectedPoints.clear();
                        selectedRenderables.insert(shapePtr);
                        if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                        {
                            selectedPoints.push_back(point);
                        }
                    }
                }
            }
        }
        ImGui::EndChild();
        const char* items[] = { "Point", "Torus", "Polyline"};
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
            selectedRenderables.clear();
            selectedPoints.clear();
        }
        ImGui::EndDisabled();
    }
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

    const float similarityThreshold = 0.02f;
    bool isCtrlPressed = ImGui::GetIO().KeyCtrl;

    for (const auto& shape : sceneRenderables)
    {
        std::shared_ptr<RenderableOnScene> shapePtr = shape;
        if (auto point = std::dynamic_pointer_cast<Point>(shape))
        {
            Algebra::Vector4 worldPos(0.f, 0.f, 0.f, 1.f);
            Algebra::Matrix4 MVP = projectionMatrix * camera.GetViewMatrix() * point->GetModelMatrix();
            Algebra::Vector4 clipPos = MVP * worldPos;

            clipPos.z = 0.f;

            if (clipPos.w != 0.f)
            {
                clipPos = clipPos / clipPos.w;
            }

            if (std::abs(ndcPos.x - clipPos.x) < similarityThreshold &&
                std::abs(ndcPos.y - clipPos.y) < similarityThreshold)
            {
                if (isCtrlPressed)
                {
                    auto it = std::find(selectedRenderables.begin(), selectedRenderables.end(), shapePtr);
                    if (it != selectedRenderables.end())
                    {
                        selectedRenderables.erase(it);
                        if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                        {
                            std::erase(selectedPoints, point);
                        }
                    }
                    else
                    {
                        selectedRenderables.insert(shapePtr);
                        if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                        {
                            selectedPoints.push_back(point);
                        }
                    }
                }
                else
                {
                    selectedRenderables.clear();
                    selectedRenderables.insert(shapePtr);
                    selectedPoints.clear();
                    if (auto point = std::dynamic_pointer_cast<Point>(shapePtr))
                    {
                        selectedPoints.push_back(point);
                    }
                }
            }
        }
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
    for (auto& renderable : sceneRenderables)
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", renderable->GetModelMatrix());
        renderable->Render();
    }

    defaultShader->SetUniformMat4f("u_modelMatrix", Algebra::Matrix4::Translation(axis.GetPosition()));
    defaultShader->SetUniformMat4f("u_viewMatrix", camera.GetRotationMatrix() * camera.GetTranslationMatrix());
    axis.Render();
    
    defaultShader->SetUniformMat4f("u_viewMatrix", camera.GetViewMatrix());
    defaultShader->SetUniformVec4f("u_color", Globals::defaultMiddlePointColor);

    if (selectedRenderables.size() != 0)
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", middleSelectionPoint.GetModelMatrix());
        middleSelectionPoint.Render();
    }

    defaultShader->UnBind();
}