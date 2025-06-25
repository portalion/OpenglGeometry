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
#include <unordered_set>
#include "UI/SelectedShapes.h"
#include <UI/ShapeCreator.h>
#include "core/Base.h"
#include "core/scene/SystemPipeline.h"

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
	void GetClickedPoint();
	Algebra::Vector4 ScreenToNDC(float x, float y);
private:
	bool running;
	bool showGrid = true;
	Window window;
	Point middleSelectionPoint;

	SelectedShapes selectedShapes;
	std::vector<std::shared_ptr<RenderableOnScene>> sceneRenderables;
	AxisCursor axis;
	InfiniteGrid grid;
	ShapeCreator shapeCreator;

	Unique<SystemPipeline> systemPipeline;
	Ref<Scene> currentScene;
};