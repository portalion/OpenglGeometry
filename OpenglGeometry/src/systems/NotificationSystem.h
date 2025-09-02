#pragma once
#include "core/Base.h"
#include "ISystem.h"

class Scene;
class Entity;

class NotificationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;

	void NotifyWithNotificationComponent(Entity entity);
	void NotifyWithVirtualComponent(Entity entity);
public:
	NotificationSystem(Ref<Scene> scene);

	void Process();
};

