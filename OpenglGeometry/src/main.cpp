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

    std::string startupScene;
    for (int i = 1; i < argc; i++)
    {
        const std::string_view arg(argv[i]);
        if (arg == "--ui-sandbox")
        {
            return UiSandbox::Run();
        }
        if (arg == "--scene" && i + 1 < argc)
        {
            startupScene = argv[++i];
        }
    }

    App& app = App::GetInstance();
    if (!startupScene.empty())
    {
        app.LoadScene(startupScene);
    }
    app.Run();

    return 0;
}
