#pragma once
#include "systems/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <scene/Entity.h>

class GUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Entity m_Cursor;
public:
	GUISystem(Ref<Scene> scene);
	void Process() override;
};
