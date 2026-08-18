#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include <unordered_set>
#include "core/Base.h"
#include "core/Viewport.h"
#include "systems/SystemPipeline.h"

class App {
public:
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App();

	static App& GetInstance();

	void Run();
private:
	App();
	Window m_Window;

	// Declared before the pipeline: the systems hold a reference to it.
	Viewport m_Viewport;

	Unique<SystemPipeline> m_SystemPipeline;
	Ref<Scene> m_CurrentScene;
};
