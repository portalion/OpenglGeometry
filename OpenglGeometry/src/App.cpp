#include "App.h"
#include <string>
#include <stdexcept>
#include <iostream>
#include "core/Globals.h"
#include <scene/BaseScene.h>

App::App()
    : window{Globals::startingSceneWidth + Globals::rightInterfaceWidth, Globals::startingSceneHeight, "Geometry"}, 
    running{true}
{
    InitImgui(window.GetWindowPointer());
    ImGui::StyleColorsDark();
    window.SetAppPointerData(this);

    HandleResize();

	currentScene = CreateRef<BaseScene>();
	systemPipeline = CreateUnique<SystemPipeline>(currentScene);
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

		if (systemPipeline)
		{
			systemPipeline->Update();
		}

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.ProcessFrame();
    }
}

void App::HandleResize()
{
	float newWidth = static_cast<float>(window.GetWidth() - Globals::rightInterfaceWidth);
	float newHeight = static_cast<float>(window.GetHeight());
	float aspect = newWidth / newHeight;
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

   }

