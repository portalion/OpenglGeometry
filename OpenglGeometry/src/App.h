#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include <unordered_set>
#include "core/Base.h"
#include "systems/SystemPipeline.h"
#include <core/Globals.h>

class App {
public:
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App();

	static App& GetInstance();

	Viewport g_Viewport;

	void Run();
private:
	App();
	Window m_Window;

	Unique<SystemPipeline> m_SystemPipeline;
	Ref<Scene> m_CurrentScene;
};