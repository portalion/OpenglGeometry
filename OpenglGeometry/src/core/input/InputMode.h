#pragma once
#include <vector>
#include <memory>
#include <core/RenderableOnScene.h>
#include <core/Camera.h>
#include <core/Window.h>
#include <unordered_set>
#include <objects/AxisCursor.h>

enum class InputModeEnum
{
	Default = 0,
	SelectedTransformation = 1,
	TransformationAroundPoint = 2,
	TransformationAroundAxis = 3,
	Size
};

class InputMode
{
protected:
	Camera* camera;
	Window* window;
	AxisCursor* cursor;
public:
	InputMode(Window* window, Camera* camera, AxisCursor* cursor);
	virtual ~InputMode() = default;
	virtual void HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems) = 0;

	static std::unique_ptr<InputMode> CreateInputMode(InputModeEnum mode, Window* window, Camera* camera, AxisCursor* cursor);
	static const std::vector<std::pair<InputModeEnum, std::string>>& GetModeList();
};

