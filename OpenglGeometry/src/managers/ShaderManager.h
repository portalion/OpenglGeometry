#pragma once
#include "engine/Shader.h"
#include <unordered_map>
#include <string>

enum class AvailableShaders
{
	Default = 0,
	InfiniteGrid = 1,
};

class ShaderManager
{
private:
	ShaderManager();

	std::unordered_map<AvailableShaders, std::shared_ptr<Shader>> shaders;

	std::shared_ptr<Shader> AddShader(AvailableShaders name, std::string filename);
	std::shared_ptr<Shader> AddShader(AvailableShaders name, std::string vertexShader, std::string fragmentShader);
public:
	~ShaderManager();
	ShaderManager(ShaderManager& other) = delete;
	void operator=(const ShaderManager&) = delete;
	
	static ShaderManager& GetInstance();

	std::shared_ptr<Shader> GetShader(AvailableShaders name);
};

