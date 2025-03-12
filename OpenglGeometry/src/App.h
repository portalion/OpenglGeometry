#pragma once
#include "utils/Initialization.h"
#include <string>
#include "Window.h"
#include "Algebra.h"

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
private:
	bool running;
	Window window;

	Algebra::Vector4 GetMousePoint(float x, float y);
	Algebra::Vector4 draggingPoint;
};