#include "SystemPipeline.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"

SystemPipeline::SystemPipeline(Ref<Scene> scene)
{
	this->scene = scene;
	renderingSystem = CreateRef<RenderingSystem>(scene);
}

SystemPipeline::~SystemPipeline()
{
}

void SystemPipeline::Update()
{
	if (renderingSystem)
	{
		//renderingSystem->Process();
	}
}
