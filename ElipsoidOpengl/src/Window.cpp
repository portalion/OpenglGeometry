#include "Window.h"
#include "utils/GlCall.h"
#include <stdexcept>

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

    glfwSetWindowUserPointer(handle, static_cast<void*>(this));
    glfwMakeContextCurrent(handle);


    HandleResize(handle, width, height);

    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int w, int h) {
        Window* tempThis = static_cast<Window*>(glfwGetWindowUserPointer(window));
        tempThis->HandleResize(window, w, h);
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

void Window::HandleResize(GLFWwindow* window, int width, int height)
{
    this->width = width;
    this->height = height;
    GLCall(glViewport(0, 0, width, height));

    glMatrixMode(GL_PROJECTION);
    float aspect = (float)width / (float)height;
    glOrtho(-aspect, aspect, -1, 1, -1, 1);
}
