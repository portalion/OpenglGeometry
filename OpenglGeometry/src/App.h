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

enum class AvailableShapes
{
	Point = 0,
	Torus = 1,
	Polyline = 2
};

class App {
public:
	App();
	~App();

	std::shared_ptr<RenderableOnScene> CreateNewShape(AvailableShapes shape);

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
	Camera camera;
	Point middleSelectionPoint;
	std::unique_ptr<InputMode> currentInputMode;

	std::unordered_set<std::shared_ptr<RenderableOnScene>> selectedRenderables;
	std::vector<std::shared_ptr<RenderableOnScene>> sceneRenderables;
	AxisCursor axis;
	InfiniteGrid grid;

	std::shared_ptr<Shader> defaultShader;
	std::vector<std::shared_ptr<Point>>selectedPoints;

	Algebra::Matrix4 projectionMatrix;
	Algebra::Matrix4 viewMatrix;
};