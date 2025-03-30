#include "InputMode.h"
#include "DefaultInputMode.h"

InputMode::InputMode(Window* window, Camera* camera)
	:window { window }, camera{ camera }
{
}

std::unique_ptr<InputMode> InputMode::CreateInputMode(InputModeEnum mode, Window* window, Camera* camera)
{
	switch (mode)
	{
	case InputModeEnum::Default:
		return std::make_unique<DefaultInputMode>(window, camera);
	}

	throw std::runtime_error("Invalid InputMode");
}
