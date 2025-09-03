#pragma once
#include <unordered_map>
#include "renderer/VertexArray.h"
#include "core/Base.h"

enum class StaticMeshType
{
	Cube,
	Square,
	Cursor
};

class StaticMeshManager
{
private:
	StaticMeshManager();
	std::unordered_map<StaticMeshType, Ref<VertexArray>> m_Meshes;

public:
	~StaticMeshManager() = default;
	StaticMeshManager(StaticMeshManager& other) = delete;
	void operator=(const StaticMeshManager&) = delete;

	static StaticMeshManager& GetInstance();

	Ref<VertexArray> GetMesh(StaticMeshType type);
};

