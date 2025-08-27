#pragma once
#include "core/Base.h"

class Scene;

class NotificationSystem
{
private:
	Ref<Scene> m_Scene;

public:
	NotificationSystem(Ref<Scene> scene);

	void Process();
};

