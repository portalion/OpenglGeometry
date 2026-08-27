#include "Initialization.h"
#include <GL/glew.h>
#include "core/Log.h"

bool InitImgui(GLFWwindow* window)
{
    bool succesful = true;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    succesful &= ImGui_ImplGlfw_InitForOpenGL(window, true);
    succesful &= ImGui_ImplOpenGL3_Init();

    return succesful;
}

bool InitGLEW()
{
    if (glewInit() != GLEW_OK)
    {
        Logger::Error("Cannot initiate glew");
        return false;
    }

    Logger::Info("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    Logger::Info("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    return true;
}
