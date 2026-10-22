#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Algebra.h"
#include "RendererContext.h"

enum class ShaderType
{
	Vertex = 1,
	Fragment = 2,
	Geometry = 3,
	TesselationControl = 4,
	TesselationEvaluation = 5,
	Compute = 6
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
	std::unordered_set<std::string> m_WarnedUniforms;
public:
	static const std::unordered_map<ShaderType, ShaderTypeInfo> shaderInfoMap;

	Shader(const std::unordered_map<ShaderType, std::string>& sourceCodes, unsigned int patchSize = 4);
	~Shader();

	void ApplyContext(UniformContext context);

	void Bind() const;
	void UnBind() const;

	// Runs a compute program; groups are counts of work groups, not invocations.
	// Bind the shader and apply its context first.
	void Dispatch(unsigned int groupsX, unsigned int groupsY = 1, unsigned int groupsZ = 1) const;
private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::unordered_map<ShaderType, std::string>& shaderSourceCodes);
	void ReflectUniforms(unsigned int program);
	int GetUniformLocation(const std::string& name);
	void WarnOnce(const std::string& uniformName, const std::string& message);

	void SetUniformMat4f(const std::string& name, const Algebra::Matrix4& matrix);
	void SetUniformVec1i(const std::string& name, const int& value);
	void SetUniformVec1ui(const std::string& name, const unsigned int& value);
	void SetUniformVec1f(const std::string& name, const float& value);
	void SetUniformVec4f(const std::string& name, const Algebra::Vector4& vector);
	void SetUniformVec2f(const std::string& name, const Algebra::Vector4& vector);
};