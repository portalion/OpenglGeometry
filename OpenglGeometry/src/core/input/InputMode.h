#pragma once
#include <vector>
#include <memory>
#include <core/RenderableOnScene.h>
#include <core/Camera.h>
#include <core/Window.h>

enum class InputModeEnum
{
	Default = 0
};

class InputMode
{
protected:
	Camera* camera;
	Window* window;
public:
	InputMode(Window* window, Camera* camera);
	virtual ~InputMode() = default;
	virtual void HandleInput(const std::vector<std::shared_ptr<RenderableOnScene>>& selectedItems) = 0;

	static std::unique_ptr<InputMode> CreateInputMode(InputModeEnum mode, Window* window, Camera* camera);
};

