#pragma once
#include "utils/Initialization.h"
#include <string>
#include "utils/Window.h"

class App {
public:
	App();
	~App();

	void Run();
private:
	bool running;
	Window window;

};