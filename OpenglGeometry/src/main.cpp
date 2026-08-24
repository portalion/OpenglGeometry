#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string_view>
#include "utils/GlCall.h"
#include "utils/Initialization.h"
#include "App.h"
#include "ui/sandbox/UiSandbox.h"

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (std::string_view(argv[i]) == "--ui-sandbox")
        {
            return UiSandbox::Run();
        }
    }

    App& app = App::GetInstance();
    app.Run();

    return 0;
}
