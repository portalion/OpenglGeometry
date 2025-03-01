#pragma once
#include "utils/Initialization.h"
#include <string>
#include "Window.h"
#include "Raycaster.h"

class App {
public:
	App();
	~App();

	void Run();
	void Render();
	void HandleInput();
	void Update();
private:
	bool running;
	Window window;

	Raycaster raycaster;
};