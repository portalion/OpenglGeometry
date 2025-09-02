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
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::TesselationControl, "default")
		.AddShader(ShaderType::TesselationEvaluation, "default")
		.AddShader(ShaderType::Fragment, "default")
		.ChangePatchSize(4));
	AssignShader(AvailableShaders::BezierSurfaceHorizontal, ShaderBuilder("resources/shaders/bezierSurface/")
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::TesselationControl, "defaultHorizontal")
		.AddShader(ShaderType::TesselationEvaluation, "defaultHorizontal")
		.AddShader(ShaderType::Fragment, "default")
		.ChangePatchSize(16));
	AssignShader(AvailableShaders::BezierSurfaceVertical, ShaderBuilder("resources/shaders/bezierSurface/")
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::TesselationControl, "defaultVertical")
		.AddShader(ShaderType::TesselationEvaluation, "defaultVertical")
		.AddShader(ShaderType::Fragment, "default")
		.ChangePatchSize(16));
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