#pragma once
#include "utils/Initialization.h"
#include <string>

struct WindowUserPointerData
{
	class App* app;
	class Window* window;
};

class Window
{
public:
	Window(int width, int height, std::string title);

	inline int GetWidth() { return width; }
	inline int GetHeight() { return height; }
	inline std::string GetTitle() { return title; }
	inline GLFWwindow* GetWindowPointer(){ return handle; }

	bool ShouldClose();
	void ProcessFrame();
	void HandleResize(int width, int height);
	void SetAppPointerData(App* app);
private:
	std::string title;
	GLFWwindow* handle;
	WindowUserPointerData data;
	int width;
	int height;
};

