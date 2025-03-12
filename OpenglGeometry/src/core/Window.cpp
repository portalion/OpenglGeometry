#include "Window.h"
#include "utils/GlCall.h"
#include <stdexcept>
#include "App.h"

Window::Window(int width, int height, std::string title)
	:width{ width }, height{ height }, title{title}
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!handle) 
    {
        throw std::runtime_error("Cannot create window");
    }

    data.window = this;

    glfwSetWindowUserPointer(handle, static_cast<void*>(&data));
    glfwMakeContextCurrent(handle);

    HandleResize(width, height);

    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int w, int h) {
        WindowUserPointerData* windowData = static_cast<WindowUserPointerData*>(glfwGetWindowUserPointer(window));
        windowData->window->HandleResize(w, h);
        windowData->app->HandleResize();
        });

    if (!InitGLEW())
    {
        throw std::runtime_error("Cannot inititate glew");
    }
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(handle);
}

void Window::ProcessFrame()
{
    glfwSwapBuffers(handle);
    glfwPollEvents();
}

void Window::HandleResize(int width, int height)
{
    this->width = width;
    this->height = height;
    GLCall(glViewport(0, 0, width, height));
}

void Window::SetAppPointerData(App* app)
{
    data.app = app;
}
