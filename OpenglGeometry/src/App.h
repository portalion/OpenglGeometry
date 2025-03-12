#pragma once
#include "utils/Initialization.h"
#include <string>
#include "Window.h"
#include "deprecated_ellipsoid/Raycaster.h"

class App {
public:
	App();
	~App();

	void Run();
	void Render();
	void HandleInput();
	void Update();
	void DisplayParameters();
private:
	bool running;
	Window window;

	RaycastableEllipsoid ellipsoid;
	Raycaster raycaster;

	Algebra::Vector4 GetMousePoint(float x, float y);
	Algebra::Vector4 draggingPoint;
};