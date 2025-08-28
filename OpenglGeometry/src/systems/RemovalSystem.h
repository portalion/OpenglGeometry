#pragma once
#include "ISystem.h"
#include "core/Base.h"
#include "scene/Scene.h"

class RemovalSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
public:
	RemovalSystem(Ref<Scene> scene);

	void Process() override;
};

