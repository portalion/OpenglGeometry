#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"

class Scene;
class Viewport;

class StatusBarSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Viewport& m_Viewport;
public:
	StatusBarSystem(Ref<Scene> scene, Viewport& viewport);

	void Process() override;
};
