#pragma once
#include <scene/Scene.h>

class MeshGeneratingSystem
{
private:
	Ref<Scene> m_Scene;

	void TorusGeneration();
public:
	MeshGeneratingSystem(Ref<Scene> m_Scene);

	void Process();
};

