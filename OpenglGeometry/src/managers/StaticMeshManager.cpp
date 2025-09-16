#include "StaticMeshManager.h"
#include <iostream>
#include <vector>
#include <chrono>
#include "Algebra.h"

StaticMeshManager::StaticMeshManager()
{
	//TODO: Make it loaded from files
	std::clog << "INFO: Loading Meshes" << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

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

	//Cursor
	{
		std::vector<float> cursorVertices =
		{
			 0.0f,  0.0f, 0.0f, 1.f,
			 0.2f,  0.0f, 0.0f, 1.f,
			 0.0f,  0.2f, 0.0f, 1.f,
			 0.0f,  0.0f, 0.2f, 1.f
		};
		std::vector<unsigned int> cursorIndices =
		{
			0, 1, 
			0, 2,
			0, 3
		};
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};
		m_Meshes[StaticMeshType::Cursor] = VertexArray::CreateWithBuffers(cursorVertices, cursorIndices, layout);
	}

	//Grid
	{
		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		const int GRID_SIZE = 200;
		const float SIZE_PER_EDGE = 2.f / GRID_SIZE;
		const float START_POS = -1.f;
		const float ENDING_POS = 1.f;

		for (int i = 0; i < GRID_SIZE + 1; i++)
		{
			Algebra::Vector4 color{ 0.2f, 0.2f, 0.2f };
			if (i == GRID_SIZE / 2.f)
			{
				color = { 1.f, 0.f, 0.f };
			}
			else if (i % 10 == 0)
			{
				color = { 0.5f, 0.5f, 0.5f };
			}

			float iCoord = START_POS + SIZE_PER_EDGE * i;
			vertices.insert(vertices.end(), 
				{
					iCoord, START_POS, 0.f, 1.f, color.x, color.y, color.z,
					iCoord, ENDING_POS, 0.f, 1.f, color.x, color.y, color.z,
					START_POS, iCoord, 0.f, 1.f, color.x, color.y, color.z,
					ENDING_POS, iCoord, 0.f, 1.f, color.x, color.y, color.z,
				});
			for(int j = 0; j < 4; j++)
				indices.push_back(indices.size());
		}
		

		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" },
			{ ShaderDataType::Float3, "color" }
		};
		m_Meshes[StaticMeshType::Grid] = VertexArray::CreateWithBuffers(vertices, indices, layout);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::clog << "INFO: Loaded Meshes - Elasped time: " << duration << std::endl;
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
