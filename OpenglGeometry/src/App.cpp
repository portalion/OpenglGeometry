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
#include "imgui/imfilebrowser.h"

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
    float fov = 3.14f / 2.f;
    float firstZ = 0.1f;
    float lastZ = 10000.f;

    projectionMatrix = Algebra::Matrix4::Projection(aspect, firstZ, lastZ, fov);
	auto matrices = StereoscopicProjection(aspect, lastZ, firstZ, fov, interocularDistance, convergenceDistance);
	projectionMatrixLeft = matrices.first;
	projectionMatrixRight = matrices.second;
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
		ImGui::Checkbox("Show stereoscopy", &drawStereo);
        ImGui::SliderFloat("Interocular Distance (d)",
            &interocularDistance,
            0.05f,
            10.f,
            "%.3f");

        ImGui::SliderFloat("Convergence Distance (c)",
            &convergenceDistance,
            2.0f,
            100.f,
            "%.2f");
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
        std::ifstream file("file_example.json");
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

void App::RenderScene()
{
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

std::pair<Algebra::Matrix4, Algebra::Matrix4> App::StereoscopicProjection(float aspect, float f, float n, float fov, float d, float c)
{
    float top = n * tanf(fov * 0.5f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    float shift = (d * 0.5f) * n / c;
    float lL = left + shift;
    float rL = right + shift;
    float lR = left - shift;
    float rR = right - shift;

    auto makeOffAxis = [&](float l, float r) {
        Algebra::Matrix4 P;

        P[0][0] = 2.0f * n / (r - l);
        P[0][2] = (r + l) / (r - l);

        P[1][1] = 2.0f * n / (top - bottom);
        P[1][2] = (top + bottom) / (top - bottom);

        P[2][2] = (f + n) / (f - n);
        P[2][3] = -2.0f * n * f / (f - n);

        P[3][2] = 1.0f;
        return P;
        };

    std::pair<Algebra::Matrix4, Algebra::Matrix4> matrices;
    matrices.first = makeOffAxis(lL, rL) * Algebra::Matrix4::Translation(d * 0.5f, 0.f, 0.f);
    matrices.second = makeOffAxis(lR, rR) * Algebra::Matrix4::Translation(-d * 0.5f, 0.f, 0.f);;

    return matrices;
}

void App::SetProjectionMatrix()
{
    HandleResize();
}

void App::SetLeftEyeProjectionMatrix()
{
    projectionMatrix = projectionMatrixLeft;
}

void App::SetRightEyeProjectionMatrix()
{
    projectionMatrix = projectionMatrixRight;
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
        HandleResize();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid.Render(camera.GetViewMatrix(), projectionMatrix, camera.GetPosition());
        glDisable(GL_BLEND);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);    // Allow depth writes
    glDisable(GL_BLEND);
    Globals::ChangeStereo(drawStereo);
    if (!drawStereo)
    {
        HandleResize();
	    RenderScene();
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
        HandleResize();
		SetLeftEyeProjectionMatrix();
        RenderScene();

        glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        SetRightEyeProjectionMatrix();
        RenderScene();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_BLEND);
    }
    if (showGrid)
    {
        HandleResize();
        glEnable(GL_BLEND);
        glDepthFunc(GL_GREATER);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid.Render(camera.GetViewMatrix(), projectionMatrix, camera.GetPosition());
    }
}