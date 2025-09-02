#include "StaticMeshManager.h"
#include <iostream>
#include <vector>

StaticMeshManager::StaticMeshManager()
{
	//TODO: Make it loaded from files
	std::clog << "INFO: Loading Meshes" << std::endl;

	//Square
	{
		std::vector<float> squareVertices =
		{
			-0.05f, -0.05f, 0.00f, 1.f,
			 0.05f,  0.05f, 0.00f, 1.f,
			-0.05f,  0.05f, 0.00f, 1.f,
	  		 0.05f, -0.05f, 0.00f, 1.f
		};
		std::vector<unsigned int> squareIndices =
		{
			0, 3, 2,
			1, 2, 3
		};
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};
		m_Meshes[StaticMeshType::Square] = VertexArray::CreateWithBuffers(squareVertices, squareIndices, layout);
	}

	//Cube
	{
		std::vector<float> cubeVertices = {
			-0.5f, -0.5f, -0.5f, 1.f,
			 0.5f, -0.5f, -0.5f, 1.f,
			 0.5f,  0.5f, -0.5f, 1.f,
			-0.5f,  0.5f, -0.5f, 1.f,
			-0.5f, -0.5f,  0.5f, 1.f,
			 0.5f, -0.5f,  0.5f, 1.f,
			 0.5f,  0.5f,  0.5f, 1.f,
			-0.5f,  0.5f,  0.5f, 1.f
		};
		std::vector<unsigned int> cubeIndices = {
			0, 1, 2,	2, 3, 0,
			4, 6, 5,	6, 4, 7,
			0, 3, 7,	7, 4, 0,
			1, 5, 6,	6, 2, 1,
			0, 4, 5,	5, 1, 0,
			3, 2, 6,	6, 7, 3
		};
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};
		m_Meshes[StaticMeshType::Cube] = VertexArray::CreateWithBuffers(cubeVertices, cubeIndices, layout);
	}

	std::clog << "INFO: Loaded Meshes" << std::endl;
}

StaticMeshManager& StaticMeshManager::GetInstance()
{
	static StaticMeshManager instance;
	
	return instance;
}

Ref<VertexArray> StaticMeshManager::GetMesh(StaticMeshType type)
{
	return m_Meshes[type];
}
