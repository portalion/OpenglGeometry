#include "App.h"
#include <string>
#include <stdexcept>
#include "engine/Shader.h"
#include <iostream>
#include "core/Globals.h"
#include <engine/Renderer.h>
#include <core/InfiniteGrid.h>
#include <objects/Polyline.h>
#include <objects/BezierCurve.h>
#include <objects/BezierCurveC2.h>
#include <objects/InterpolatedBezierCurve.h>

Algebra::Matrix4 App::projectionMatrix;
Camera App::camera = Camera(Globals::startingCameraPosition, 1.f);
Window* App::windowStatic = nullptr;
App::App()
    : window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}, 
    running{true},
	shapeCreator{ &selectedShapes, &axis }
{
    InitImgui(window.GetWindowPointer());
    window.SetAppPointerData(this);
	windowStatic = &window;
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
    for (auto& renderable : sceneRenderables)
    {
        renderable->Update();
    }
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
    this->CreateShape();
    if (ImGui::CollapsingHeader("Selected items parameters", ImGuiTreeNodeFlags_Leaf))
    {
        for (auto& renderable : selectedShapes.GetSelectedWithType<RenderableOnScene>())
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
                if (ImGui::Selectable(renderable->GenerateLabelWithId(renderable->GetName()).c_str(), selectedShapes.IsSelected(renderable)))
                {
                    if (ctrlPressed)
                    {
                        selectedShapes.ToggleShape(renderable);
                    }
                    else
                    {
                        selectedShapes.Clear();
						selectedShapes.AddShape(renderable);
                    }
                }
            }
        }
        ImGui::EndChild();
        auto availableShapes = ShapeCreator::GetShapeList();
		static int currentShapeIndex = 0;

        if (ImGui::Button("Create shape"))
        {
            auto newShape = shapeCreator.CreateShape(availableShapes[currentShapeIndex].first);
            newShape->InitName();
            newShape->Move(axis.GetPosition());
            sceneRenderables.push_back(newShape);

            if (availableShapes[currentShapeIndex].first == ShapeEnum::Point)
            {
				auto beziers = selectedShapes.GetSelectedWithType<BezierCurve>();
                for (auto bezier : beziers)
                {
					bezier->AddPoint(std::dynamic_pointer_cast<Point>(newShape));
                }
                auto beziers2 = selectedShapes.GetSelectedWithType<BezierCurveC2>();
                for (auto bezier : beziers2)
                {
                    bezier->AddPoint(std::dynamic_pointer_cast<Point>(newShape));
                }
                auto interpolatedBeziers = selectedShapes.GetSelectedWithType<InterpolatedBezierCurve>();
                for (auto bezier : interpolatedBeziers)
                {
                    bezier->AddPoint(std::dynamic_pointer_cast<Point>(newShape));
                }
            }

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Globals::rightInterfaceWidth / 3.f);
		if (ImGui::BeginCombo("##shape", availableShapes[currentShapeIndex].second.c_str()))
		{
			for (int i = 0; i < availableShapes.size(); ++i)
			{
				const bool isSelected = (currentShapeIndex == i);
				if (ImGui::Selectable(availableShapes[i].second.c_str(), isSelected))
				{
					currentShapeIndex = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
        ImGui::SameLine();

        ImGui::BeginDisabled(selectedShapes.IsEmpty());
        if (ImGui::Button("Remove shapes"))
        {
            sceneRenderables.erase(
                std::remove_if(sceneRenderables.begin(), sceneRenderables.end(),
                    [&](const std::shared_ptr<RenderableOnScene>& shape) {
                        bool shouldRemove = selectedShapes.IsSelected(shape);
                        return shouldRemove; 
                    }),
                sceneRenderables.end()
            );
            selectedShapes.Clear();
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
					selectedShapes.ToggleShape(shapePtr);
                }
                else
                {
                    selectedShapes.Clear();
                    selectedShapes.AddShape(shapePtr);
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

    if (!selectedShapes.IsEmpty())
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", middleSelectionPoint.GetModelMatrix());
        middleSelectionPoint.Render();
    }
  
    defaultShader->UnBind();
}