#include "Shader.h"

#include <iostream>
#include "utils/GlCall.h"

const std::unordered_map<ShaderType, ShaderTypeInfo> Shader::shaderInfoMap =
{
    { ShaderType::Vertex, ShaderTypeInfo{.fileExtension = ".vert", .glShaderId = GL_VERTEX_SHADER }},
    { ShaderType::Fragment, ShaderTypeInfo{.fileExtension = ".frag", .glShaderId = GL_FRAGMENT_SHADER }},
    { ShaderType::Geometry, ShaderTypeInfo{.fileExtension = ".geom", .glShaderId = GL_GEOMETRY_SHADER }},
    { ShaderType::TesselationControl, ShaderTypeInfo{.fileExtension = ".tesc", .glShaderId = GL_TESS_CONTROL_SHADER }},
    { ShaderType::TesselationEvaluation, ShaderTypeInfo{.fileExtension = ".tese", .glShaderId = GL_TESS_EVALUATION_SHADER }},
};

Shader::Shader(const std::unordered_map<ShaderType, std::string>& sourceCodes)
    :m_RendererID{ 0 }
{
    m_RendererID = CreateShader(sourceCodes);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::SetUniformMat4f(const std::string& name, const Algebra::Matrix4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_TRUE, &matrix[0][0]));
}

void Shader::SetUniformVec4f(const std::string& name, const Algebra::Vector4& vector)
{
    GLCall(glUniform4f(GetUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::SetUniformVec2f(const std::string& name, const Algebra::Vector4& vector)
{
    GLCall(glUniform2f(GetUniformLocation(name), vector.x, vector.y));
}

void Shader::SetUniformVec1i(const std::string& name, const int& value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        printf("Failed to compile %s shader!\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::unordered_map<ShaderType, std::string>& shaderSourceCodes)
{
    unsigned int program = glCreateProgram();

    for (auto& shaderSourceCode : shaderSourceCodes)
    {
        auto glType = shaderInfoMap.at(shaderSourceCode.first).glShaderId;
        unsigned int compiledShader = CompileShader(glType, shaderSourceCode.second);
        GLCall(glAttachShader(program, compiledShader));
        GLCall(glDeleteShader(compiledShader));
    }

    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::UnBind() const
{
    GLCall(glUseProgram(0));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(unsigned int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1) printf("Warning: uniform '%s' doesn't exist!\n", name.c_str());
    else m_UniformLocationCache[name] = location;

    return location;
}