#include "ShaderBuilder.h"

#include <iostream>
#include <sstream>
#include <fstream>

std::stringstream ShaderBuilder::ParseShaderCode(const std::string& filepath)
{
    std::ifstream stream(filepath);

    std::string line;
    std::stringstream ss;
    while (getline(stream, line))
    {
        ss << line << '\n';
    }

    return ss;
}

ShaderBuilder::ShaderBuilder(const std::string& resourceDirectory)
    :resourceDirectory{ resourceDirectory }
{
}

ShaderBuilder& ShaderBuilder::AddShader(ShaderType type, const std::string& filename)
{
    if (shaderSourceCodes.find(type) != shaderSourceCodes.end())
    {
        std::cout << "WARNING: shader with type: " << static_cast<int>(type) << " already existing.\n";
    }

    std::string filepath = resourceDirectory + filename + Shader::shaderInfoMap.at(type).fileExtension;

    shaderSourceCodes[type] = ParseShaderCode(filepath).str();

    return *this;
}

ShaderBuilder& ShaderBuilder::ChangePatchSize(unsigned int patchSize)
{
    this->patchSize = patchSize;
    return *this;
}

Shader ShaderBuilder::Build() const
{
    return Shader(shaderSourceCodes, patchSize);
}

std::shared_ptr<Shader> ShaderBuilder::BuildShared() const
{
    return std::make_shared<Shader>(shaderSourceCodes, patchSize);
}