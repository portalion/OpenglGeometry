#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include "core/Camera.h"
#include <unordered_set>
#include "core/Base.h"
#include "systems/SystemPipeline.h"

class App {
public:
	App();
	~App();

	void Run();
	void Render();
	void HandleInput();
	void HandleResize();
	void Update();
	void DisplayParameters();
	void CreateShape();
	void GetClickedPoint();
	Algebra::Vector4 ScreenToNDC(float x, float y);
private:
	bool running;
	bool showGrid = true;
	Window window;


	Unique<SystemPipeline> systemPipeline;
	Ref<Scene> currentScene;
};