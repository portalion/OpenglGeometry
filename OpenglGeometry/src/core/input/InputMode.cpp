#include "InputMode.h"
#include "DefaultInputMode.h"
#include "SelectedTransformationInputMode.h"

InputMode::InputMode(Window* window, Camera* camera, AxisCursor* cursor)
	:window{ window }, camera{ camera }, cursor{ cursor }
{
}

std::unique_ptr<InputMode> InputMode::CreateInputMode(InputModeEnum mode, Window* window, Camera* camera, AxisCursor* cursor)
{
	switch (mode)
	{
	case InputModeEnum::Default:
		return std::make_unique<DefaultInputMode>(window, camera, cursor);
	case InputModeEnum::SelectedTransformation:
		return std::make_unique<SelectedTransformationInputMode>(window, camera, cursor);
	}

	throw std::runtime_error("Invalid InputMode");
}

const std::vector<std::pair<InputModeEnum, std::string>>& InputMode::GetModeList() 
{
	static const std::vector<std::pair<InputModeEnum, std::string>> modeList = {
		{ InputModeEnum::Default, "Default" },
		{ InputModeEnum::SelectedTransformation, "Selected Transformation" },
	};
	return modeList;
}