#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "core/InfiniteGrid.h"
#include "objects/Torus.h"


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
	bool showGrid = true;
	Window window;
	Camera camera;
	RenderableOnScene* torus;
	InfiniteGrid grid;
	Shader* defaultShader;

	Algebra::Vector4 GetMousePoint(float x, float y);
	Algebra::Vector4 draggingPoint;

	Algebra::Matrix4 projectionMatrix;
	Algebra::Matrix4 viewMatrix;
};