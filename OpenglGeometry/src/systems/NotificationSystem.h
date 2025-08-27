#pragma once
#include "core/Base.h"
#include "ISystem.h"

class Scene;

class NotificationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;

public:
	NotificationSystem(Ref<Scene> scene);

	void Process();
};

