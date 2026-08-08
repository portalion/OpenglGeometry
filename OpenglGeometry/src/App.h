#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include <unordered_set>
#include "core/Base.h"
#include "systems/SystemPipeline.h"

class App {
public:
	App();
	~App();

	void Run();
	void HandleResize();
private:
	bool running;
	Window window;

	Unique<SystemPipeline> systemPipeline;
	Ref<Scene> currentScene;
};