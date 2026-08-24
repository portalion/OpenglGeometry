#pragma once
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <scene/Entity.h>
#include <core/Viewport.h>
#include <ui/core/DockSpace.h>
#include <ui/ShapeInspectorRegistry.h>
#include <ui/ShapeList.h>
#include <ui/Utils.h>
#include <ui/model/UiState.h>

class GUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Viewport& m_Viewport;
	Dockspace m_Dockspace;
	bool m_ShowImGuiDemo = false;
	UiState m_UiState;

	GUI::ShapeList m_ShapeList;
	ShapeInspectorRegistry m_ShapeInspector;

	GUI::UICallbacks m_Callbacks;
public:
	GUISystem(Ref<Scene> scene, Viewport& viewport);
	void Process() override;
};
