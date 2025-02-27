#include "App.h"
#include <string>

App::App()
    : currentWindow{nullptr}, running{true}
{
    running &= InitializeWindow(640, 480, "temp");
    running &= InitImgui(currentWindow);
    running &= InitGLEW();
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
    while (running && !glfwWindowShouldClose(currentWindow))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        /* Swap front and back buffers */
        glfwSwapBuffers(currentWindow);

        /* Poll for and process events */
        glfwPollEvents();
        // Rendering
    }
}

bool App::InitializeWindow(const int width, const int height, const std::string& title)
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    currentWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!currentWindow)
    {
        glfwTerminate();
        return false;
    }

    glViewport(0, 0, width, height);

    glfwMakeContextCurrent(currentWindow);

	return false;
}
