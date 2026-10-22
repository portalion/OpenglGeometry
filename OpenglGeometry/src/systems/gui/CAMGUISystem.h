#pragma once
#include <string>
#include <vector>
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <core/Viewport.h>
#include <ui/core/DockSpace.h>
#include <scene/CAMSettings.h>

class CAMGUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<CAMSettings> m_Settings;
	Viewport& m_Viewport;
	Dockspace m_Dockspace{ "CAMDockspace" };
	std::vector<std::string> m_PathFiles;
	std::vector<std::string> m_BrowsePaths;

	void DrawMenuBar();
	void DrawPathPanel();
	void DrawLoadedPaths();
	void DrawPathControls(ID pathId);
	void DrawMaterialSection();
	void DrawSimulationSection();
	void LoadPath(const std::string& file);
public:
	CAMGUISystem(Ref<Scene> scene, Ref<CAMSettings> settings, Viewport& viewport);
	void Process() override;
};
