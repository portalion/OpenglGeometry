#pragma once
#include "utils/Initialization.h"
#include <string>

class App {
public:
	App();
	~App();

	void Run();
private:
	bool running;
	GLFWwindow* currentWindow;

	bool InitializeWindow(const int width, const int height, const std::string& title);
};