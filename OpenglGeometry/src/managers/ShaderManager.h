#pragma once
#include "renderer/Shader.h"
#include <unordered_map>
#include <string>

enum class AvailableShaders
{
	Default = 0,
	InfiniteGrid = 1,
	BezierCurveC0 = 2,
};

class ShaderBuilder;

class ShaderManager
{
private:
	ShaderManager();

	std::unordered_map<AvailableShaders, std::shared_ptr<Shader>> shaders;

	std::shared_ptr<Shader> AssignShader(AvailableShaders name, const ShaderBuilder& builder);
public:
	~ShaderManager() = default;
	ShaderManager(ShaderManager& other) = delete;
	void operator=(const ShaderManager&) = delete;
	
	static ShaderManager& GetInstance();

	std::shared_ptr<Shader> GetShader(AvailableShaders name);
};

