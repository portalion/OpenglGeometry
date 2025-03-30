#pragma once
#include <string>
#include "utils/Initialization.h"
#include "Algebra.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "core/InfiniteGrid.h"
#include "objects/Torus.h"
#include "objects/AxisCursor.h"
#include <objects/Point.h>
#include <core/input/InputMode.h>
#include <unordered_set>


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
private:
	bool running;
	bool showGrid = true;
	Window window;
	Camera camera;
	std::unique_ptr<InputMode> currentInputMode;

	std::unordered_set<std::shared_ptr<RenderableOnScene>> selectedRenderables;
	std::vector<std::shared_ptr<RenderableOnScene>> sceneRenderables;
	AxisCursor axis;
	InfiniteGrid grid;

	std::shared_ptr<Shader> defaultShader;

	Algebra::Matrix4 projectionMatrix;
	Algebra::Matrix4 viewMatrix;
};