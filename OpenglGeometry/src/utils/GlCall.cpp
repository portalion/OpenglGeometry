#include "glCall.h"
#include "core/Log.h"

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        Logger::Error("[OpenGL Error] ({:#x}): {} {}:{}", error, function, file, line);
        return false;
    }

    return true;
}
