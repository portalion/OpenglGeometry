#include "ShaderManager.h"
#include <iostream>
#include "engine/ShaderBuilder.h"

ShaderManager::ShaderManager()
{
	//TODO: Change into logger
	std::cout << "INFO: Loading Shaders" << std::endl;
	AssignShader(AvailableShaders::Default, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::Fragment, "default"));
	AssignShader(AvailableShaders::InfiniteGrid, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "infiniteGrid")
		.AddShader(ShaderType::Fragment, "infiniteGrid"));
	AssignShader(AvailableShaders::BezierLine, ShaderBuilder("resources/shaders/bezierLine")
		.AddShader(ShaderType::Vertex, "bezier")
		.AddShader(ShaderType::TesselationControl, "bezier")
		.AddShader(ShaderType::TesselationEvaluation, "bezier")
		.AddShader(ShaderType::Fragment, "bezier")
		.ChangePatchSize(4)); 
	AssignShader(AvailableShaders::BezierSurface, ShaderBuilder("resources/shaders/bezierSurface")
		.AddShader(ShaderType::Vertex, "bezier")
		.AddShader(ShaderType::TesselationControl, "bezier")
		.AddShader(ShaderType::TesselationEvaluation, "bezier")
		.AddShader(ShaderType::Fragment, "bezier")
		.ChangePatchSize(16));
	std::cout << "INFO: Loaded Shaders" << std::endl;
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