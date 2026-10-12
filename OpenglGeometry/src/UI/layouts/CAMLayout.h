#pragma once
#include <ui/core/DockSpace.h>

namespace GUI
{
	inline constexpr const char* CAMTopPanelWindow = "Top##CAM Top Panel";
	inline constexpr const char* CAMMiddlePanelWindow = "Middle##CAM Middle Panel";
	inline constexpr const char* CAMBottomPanelWindow = "Bottom##CAM Bottom Panel";

	namespace Layout
	{
		void CAM(Dockspace& dockspace);
	}
}
