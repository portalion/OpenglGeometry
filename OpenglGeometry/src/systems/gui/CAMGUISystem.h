#pragma once
#include <string>
#include "interfaces/ISystem.h"
#include "core/Base.h"
#include <scene/Scene.h>
#include <core/Viewport.h>
#include <ui/core/DockSpace.h>

class CAMGUISystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Viewport& m_Viewport;
	Dockspace m_Dockspace{ "CAMDockspace" };
	std::string m_PathFile;
	std::string m_BrowsePath;

	void DrawMenuBar();
	void DrawPathPanel();
	void LoadPath(const std::string& file);
public:
	CAMGUISystem(Ref<Scene> scene, Viewport& viewport);
	void Process() override;
};
