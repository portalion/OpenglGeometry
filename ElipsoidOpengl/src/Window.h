#pragma once
#include "utils/Initialization.h"
#include <string>

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
private:
	std::string title;
	GLFWwindow* handle;
	int width;
	int height;

	void HandleResize(GLFWwindow* window, int width, int height);
};

