#include "ShaderManager.h"
#include <iostream>

ShaderManager::ShaderManager()
{
	AddShader(AvailableShaders::Default, "resources/shaders/default");
	AddShader(AvailableShaders::InfiniteGrid, "resources/shaders/infiniteGrid");
}

ShaderManager& ShaderManager::GetInstance()
{
	static ShaderManager instance;

	return instance;
}

std::shared_ptr<Shader> ShaderManager::GetShader(AvailableShaders name)
{
	return shaders[name];
}

std::shared_ptr<Shader> ShaderManager::AddShader(AvailableShaders name, std::string filename)
{
	if (shaders.find(name) != shaders.end())
	{
		std::cerr << "WARNING: shader with name: " << static_cast<int>(name) << " already existing\n";
		return shaders[name];
	}
	shaders[name] = std::make_shared<Shader>(filename);
	return shaders[name];
}

std::shared_ptr<Shader> ShaderManager::AddShader(AvailableShaders name, std::string vertexShader, std::string fragmentShader)
{
	if (shaders.find(name) != shaders.end())
	{
		std::cerr << "WARNING: shader with name: " << static_cast<int>(name) << " already existing\n";
		return shaders[name];
	}
	shaders[name] = std::make_shared<Shader>(vertexShader, fragmentShader);
	return shaders[name];
}

ShaderManager::~ShaderManager()
{
}
