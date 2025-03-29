#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_stdlib.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

bool InitImgui(GLFWwindow* window);
bool InitGLEW();