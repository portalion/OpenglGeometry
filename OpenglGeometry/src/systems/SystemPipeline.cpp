#include "SystemPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

#include "RenderingSystem.h"
#include "MeshGeneratingSystem.h"
#include "NotificationSystem.h"
#include "RemovalSystem.h"
#include "SelectionMarkerSystem.h"
#include "CurveBernsteinSystem.h"
#include "gui/GUISystem.h"
#include "gui/PopupSystem.h"
#include <core/Globals.h>

SystemPipeline::SystemPipeline(Ref<Scene> m_Scene, Viewport& viewport)
{
	this->m_Scene = m_Scene;

	m_Systems.push_back(CreateUnique<NotificationSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<RemovalSystem>(m_Scene));

	m_Systems.push_back(CreateUnique<GUISystem>(m_Scene, viewport));
	m_Systems.push_back(CreateUnique<PopupSystem>(m_Scene));

	m_Systems.push_back(CreateUnique<CurveBernsteinSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<MeshGeneratingSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<SelectionMarkerSystem>(m_Scene));
	m_Systems.push_back(CreateUnique<RenderingSystem>(m_Scene, viewport));
}

SystemPipeline::~SystemPipeline()
{
}

void SystemPipeline::Update()
{
	for(auto& system : m_Systems)
	{
		system->Process();
	}
}
