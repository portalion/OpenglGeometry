#pragma once
#include <unordered_map>
#include <string>
#include "Shader.h"

class ShaderBuilder
{
private:
	std::unordered_map<ShaderType, std::string> shaderSourceCodes;
	std::string resourceDirectory;

	std::stringstream ParseShaderCode(const std::string& filename);
public:
	ShaderBuilder(const std::string& resourceDirectory);
	ShaderBuilder& AddShader(ShaderType type, const std::string& filename);

	Shader Build() const;
	std::shared_ptr<Shader> BuildShared() const;
};