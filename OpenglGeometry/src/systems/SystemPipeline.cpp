#include "SystemPipeline.h"
#include "scene/Scene.h"

#include "RenderingSystem.h"
#include "MeshGeneratingSystem.h"
#include "NotificationSystem.h"
#include "RemovalSystem.h"
#include "SelectionMarkerSystem.h"
#include "CurveBernsteinSystem.h"
#include "simulationSystem/SimulationSystem.h"
#include "gui/GUISystem.h"
#include "gui/CAMGUISystem.h"
#include "gui/PopupSystem.h"
#include "gui/LoggerSystem.h"
#include "gui/StatusBarSystem.h"
#include "scene/CAMSettings.h"

void SystemPipeline::Update()
{
	for(auto& system : m_Systems)
	{
		system->Process();
	}
}

ModellingSystemPipeline::ModellingSystemPipeline(Ref<Scene> m_Scene, Viewport& viewport)
{
	m_Systems.push_back(CreateUnique<NotificationSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<RemovalSystem>(m_Scene));

	m_Systems.push_back(CreateUnique<GUISystem>(m_Scene, viewport));
	m_Systems.push_back(CreateUnique<PopupSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<LoggerSystem>());
	m_Systems.push_back(CreateUnique<StatusBarSystem>(m_Scene, viewport));

	m_Systems.push_back(CreateUnique<CurveBernsteinSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<MeshGeneratingSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<SelectionMarkerSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<RenderingSystem>(m_Scene, viewport));
}

CAMSystemPipeline::CAMSystemPipeline(Ref<Scene> m_Scene, Viewport& viewport)
{
	auto settings = CreateRef<CAMSettings>();

	m_Systems.push_back(CreateUnique<CAMGUISystem>(m_Scene, settings, viewport));
	m_Systems.push_back(CreateUnique<LoggerSystem>());
	m_Systems.push_back(CreateUnique<StatusBarSystem>(m_Scene, viewport));

	m_Systems.push_back(CreateUnique<SimulationSystem>(m_Scene, settings));
	m_Systems.push_back(CreateUnique<RenderingSystem>(m_Scene, viewport));
}
