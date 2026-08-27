#include "ShaderManager.h"
#include "renderer/ShaderBuilder.h"
#include "core/Log.h"

ShaderManager::ShaderManager()
{
	Logger::Info("Loading shaders");
	AssignShader(AvailableShaders::Default, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "default")
		.AddShader(ShaderType::Fragment, "default"));
	AssignShader(AvailableShaders::InfiniteGrid, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "infiniteGrid")
		.AddShader(ShaderType::Fragment, "infiniteGrid"));
	AssignShader(AvailableShaders::Point, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "point")
		.AddShader(ShaderType::Fragment, "default"));
	AssignShader(AvailableShaders::Cursor, ShaderBuilder("resources/shaders/")
		.AddShader(ShaderType::Vertex, "cursor")
		.AddShader(ShaderType::Fragment, "cursor"));
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
	Logger::Info("Loaded shaders");
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
		Logger::Warning("shader with name: {} already existing", static_cast<int>(name));
		return shaders[name];
	}
	shaders[name] = builder.BuildShared();
	return shaders[name];
}