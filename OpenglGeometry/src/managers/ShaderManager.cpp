#include "ShaderManager.h"
#include <iostream>
#include "renderer/ShaderBuilder.h"

ShaderManager::ShaderManager()
{
	//TODO: Change into logger
	std::clog << "INFO: Loading Shaders" << std::endl;
	AssignShader(AvailableShaders::Default, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::Fragment, "default"));
	AssignShader(AvailableShaders::InfiniteGrid, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "infiniteGrid")
		.AddShader(ShaderType::Fragment, "infiniteGrid"));
	AssignShader(AvailableShaders::BezierCurveC0, ShaderBuilder("resources/shaders/bezierLine/")
		.AddShader(ShaderType::Vertex, "bezierC0")
		.AddShader(ShaderType::TesselationControl, "bezierC0")
		.AddShader(ShaderType::TesselationEvaluation, "bezierC0")
		.AddShader(ShaderType::Fragment, "bezierC0")
		.ChangePatchSize(4));
	std::clog << "INFO: Loaded Shaders" << std::endl;
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

std::shared_ptr<Shader> ShaderManager::AssignShader(AvailableShaders name, const ShaderBuilder& builder)
{
	if (shaders.find(name) != shaders.end())
	{
		std::cerr << "WARNING: shader with name: " << static_cast<int>(name) << " already existing\n";
		return shaders[name];
	}
	shaders[name] = builder.BuildShared();
	return shaders[name];
}