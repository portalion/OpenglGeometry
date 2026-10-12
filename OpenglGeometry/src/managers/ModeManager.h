#pragma once
#include <vector>
#include "core/Base.h"
#include "systems/SystemPipeline.h"

class ModeManager
{
private:
	std::vector<Unique<SystemPipeline>> m_Systems;
	Ref<Scene> m_CurrentScene;
	Viewport& m_Viewport;
	int m_CurrentModeIndex = 0;
public:
	ModeManager(Ref<Scene> scene, Viewport& viewport);
	
	void ChangeMode();
	bool ShouldChange();
	void Update();
};