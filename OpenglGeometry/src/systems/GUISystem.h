#pragma once
#include "systems/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>

class GUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
public:
	GUISystem(Ref<Scene> scene);
	void Process() override;
};
