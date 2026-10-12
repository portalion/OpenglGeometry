#pragma once
#include <string>
#include "core/Window.h"
#include "core/Base.h"
#include "core/Viewport.h"
#include "managers/ModeManager.h"

class App {
public:
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App();

	static App& GetInstance();

	void Run();
	void LoadScene(const std::string& path);
private:
	App();
	Window m_Window;

	// Declared before the pipeline: the systems hold a reference to it.
	Viewport m_Viewport;

	Unique<ModeManager> m_ModeManager;

	Ref<Scene> m_CurrentScene;
};
