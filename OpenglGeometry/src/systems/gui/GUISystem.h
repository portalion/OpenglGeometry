#pragma once
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <scene/Entity.h>
#include <core/Viewport.h>
#include <ui/core/DockSpace.h>
#include <ui/CursorControl.h>
#include <ui/Inspector.h>
#include <ui/ShapeList.h>
#include <ui/Utils.h>
#include <ui/ViewportPicking.h>
#include <ui/model/UiState.h>

class GUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Viewport& m_Viewport;
	Dockspace m_Dockspace;
	bool m_ShowImGuiDemo = false;
	bool m_ShowParameterSpace = false;
	UiState m_UiState;

	GUI::ShapeList m_ShapeList;

	GUI::ViewportPickingState m_Picking;
	GUI::CursorPlacementState m_CursorPlacement;

	GUI::UICallbacks m_Callbacks;
	GUI::InspectorCallbacks m_InspectorCallbacks;

	void SyncInspectorState();
	void WriteBackInspectorState();
public:
	GUISystem(Ref<Scene> scene, Viewport& viewport);
	void Process() override;
};
