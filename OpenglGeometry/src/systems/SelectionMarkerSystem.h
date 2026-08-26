#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"

class Scene;

class SelectionMarkerSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
public:
	SelectionMarkerSystem(Ref<Scene> scene);

	void Process() override;
};
