#pragma once

#include <string>
#include <unordered_map>
#include "Algebra.h"
#include "RendererContext.h"

enum class ShaderType
{
	Vertex = 1,
	Fragment = 2,
	Geometry = 3,
	TesselationControl = 4,
	TesselationEvaluation = 5
};

struct ShaderTypeInfo
{
	std::string fileExtension;
	unsigned int glShaderId;
};

struct UniformInfo;

class Shader
{
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	std::unordered_map<std::string, int> m_UniformLocationCache;
	unsigned int patchSize;
	std::vector<UniformInfo> m_Uniforms;
public:
	static const std::unordered_map<ShaderType, ShaderTypeInfo> shaderInfoMap;

	Shader(const std::unordered_map<ShaderType, std::string>& sourceCodes, unsigned int patchSize = 4);
	~Shader();

	void ApplyContext(UniformContext context);

	void Bind() const;
	void UnBind() const;
private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::unordered_map<ShaderType, std::string>& shaderSourceCodes);
	void ReflectUniforms(unsigned int program);
	int GetUniformLocation(const std::string& name);

	void SetUniformMat4f(const std::string& name, const Algebra::Matrix4& matrix);
	void SetUniformVec1i(const std::string& name, const int& value);
	void SetUniformVec4f(const std::string& name, const Algebra::Vector4& vector);
	void SetUniformVec2f(const std::string& name, const Algebra::Vector4& vector);
};