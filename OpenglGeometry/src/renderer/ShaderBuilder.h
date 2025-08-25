#pragma once
#include <unordered_map>
#include <string>
#include "Shader.h"

class ShaderBuilder
{
private:
	std::unordered_map<ShaderType, std::string> shaderSourceCodes;
	std::string resourceDirectory;
	unsigned int patchSize = 4;

	std::stringstream ParseShaderCode(const std::string& filename);
public:
	ShaderBuilder(const std::string& resourceDirectory);
	ShaderBuilder& AddShader(ShaderType type, const std::string& filename);
	ShaderBuilder& ChangePatchSize(unsigned int patchSize);

	Shader Build() const;
	std::shared_ptr<Shader> BuildShared() const;
};