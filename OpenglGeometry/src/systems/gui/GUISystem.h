#pragma once
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <scene/Entity.h>
#include <ui/core/DockSpace.h>
#include <ui/ShapeInspectorRegistry.h>

class GUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Dockspace m_Dockspace;
	bool m_ShowImGuiDemo = false;

	ShapeInspectorRegistry m_ShapeInspector;
public:
	GUISystem(Ref<Scene> scene);
	void Process() override;
};
