#include "MeshGeneratingSystem.h"

void MeshGeneratingSystem::TorusGeneration()
{
}

MeshGeneratingSystem::MeshGeneratingSystem(Ref<Scene> m_Scene)
	:m_Scene {m_Scene}
{
}

void MeshGeneratingSystem::Process()
{
	TorusGeneration();
}
