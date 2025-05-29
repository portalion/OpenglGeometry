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
#include "imgui/imfilebrowser.h"

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
	ImGui::FileBrowser openFileBrowser;
	ImGui::FileBrowser saveFileBrowser;
	bool dialogOpen = false;
	bool dialogSave = false;
	bool running;
	bool showGrid = true;
	Window window;

	Point middleSelectionPoint;
	std::unique_ptr<InputMode> currentInputMode;

	SelectedShapes selectedShapes;
	AxisCursor axis;
	InfiniteGrid grid;
	ShapeList shapeList;

	Algebra::Matrix4 projectionMatrixLeft;
	Algebra::Matrix4 projectionMatrixRight;
	float interocularDistance = 0.05f, convergenceDistance = 2.f;

	bool drawStereo = false;
	void RenderScene();

	std::shared_ptr<Shader> defaultShader;
	void SetProjectionMatrix();
	void SetLeftEyeProjectionMatrix();
	void SetRightEyeProjectionMatrix();
	std::pair<Algebra::Matrix4, Algebra::Matrix4> StereoscopicProjection(float aspect, float f, float n, float fov, float d, float c);

};