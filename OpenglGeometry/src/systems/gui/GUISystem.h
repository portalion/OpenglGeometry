#pragma once
#include <unordered_map>
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <Algebra.h>
#include <scene/Scene.h>
#include <scene/Entity.h>
#include <core/Viewport.h>
#include <ui/core/DockSpace.h>
#include <ui/CursorControl.h>
#include <ui/CursorPanel.h>
#include <ui/Inspector.h>
#include <ui/ShapeList.h>
#include <ui/Utils.h>
#include <ui/ViewportPicking.h>
#include <ui/model/UiState.h>

class GUISystem : public ISystem
{
private:
	struct RotationEdit
	{
		Algebra::Vector4 euler = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
		Algebra::Quaternion applied;
		bool seeded = false;
	};

	Ref<Scene> m_Scene;
	Viewport& m_Viewport;
	Dockspace m_Dockspace;
	bool m_ShowImGuiDemo = false;
	bool m_ShowParameterSpace = false;
	bool m_ShowLog = true;
	UiState m_UiState;

	GUI::ShapeList m_ShapeList;

	GUI::ViewportPickingState m_Picking;
	GUI::CursorPlacementState m_CursorPlacement;

	GUI::UICallbacks m_Callbacks;
	GUI::CursorPanelCallbacks m_CursorPanelCallbacks;

	std::unordered_map<uint32_t, RotationEdit> m_RotationEdits;

	void SyncInspectorState();
	void WriteBackInspectorState();
	void WriteBackRotation(Entity entity, uint32_t id, const Algebra::Vector4& editedEuler);
public:
	GUISystem(Ref<Scene> scene, Viewport& viewport);
	void Process() override;
};
