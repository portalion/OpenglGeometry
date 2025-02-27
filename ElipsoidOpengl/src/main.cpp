#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "utils/GlCall.h"
#include "utils/Initialization.h"
#include "App.h"

int main(void)
{
    if (!glfwInit()) {
        return -1;
    }

    App app;
    app.Run();

    return 0;
}