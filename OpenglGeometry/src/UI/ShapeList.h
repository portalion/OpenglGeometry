#pragma once
#include <vector>
#include <core/RenderableOnScene.h>
#include <objects/Point.h>
#include <memory>
#include "SelectedShapes.h"
#include "ShapeCreator.h"

class ShapeList
{
private:
	SelectedShapes* selectedShapes;
	ShapeCreator shapeCreator;
	std::vector<std::shared_ptr<RenderableOnScene>> shapes;
	std::shared_ptr<Shader> defaultShader;
	bool isInCreationMode = false;
	bool createC2 = false;
	bool isCylinder = false;

	int rPatches = 1;
	int yPatches = 1;
	float sizeR = 10.f;
	float sizeY = 10.f;

	void CreateShapeButton();
	void ShowCreationPopup();
public:
	ShapeList(AxisCursor* axis, SelectedShapes* selectedShapes);
	
	std::shared_ptr<Point> GetPointByPosition(Algebra::Matrix4 VP, Algebra::Vector4 position) const;

	void StartCreationMode(bool C2);
	void DisplayUI();
	void Update();
	void Render();
	void AddPoint(std::shared_ptr<Point> point);
	void RemovePoint(std::shared_ptr<Point> point);

	json SerializeList();
};

