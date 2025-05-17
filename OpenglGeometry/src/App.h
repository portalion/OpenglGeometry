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
#include "UI/SelectedShapes.h"
#include <UI/ShapeCreator.h>
#include <UI/ShapeList.h>

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
	void GetClickedPoint();
	Algebra::Vector4 ScreenToNDC(float x, float y);
	static Algebra::Matrix4 projectionMatrix;
	static Camera camera;
	static Window* windowStatic;
private:
	bool running;
	bool showGrid = true;
	Window window;

	Point middleSelectionPoint;
	std::unique_ptr<InputMode> currentInputMode;

	SelectedShapes selectedShapes;
	AxisCursor axis;
	InfiniteGrid grid;
	ShapeList shapeList;

	std::shared_ptr<Shader> defaultShader;
};