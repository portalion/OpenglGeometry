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

	{
		std::vector<float> cursorVertices =
		{
			 0.0f,  0.0f,  0.0f, 1.f,     1.f, 0.f, 0.f,
			 0.2f,  0.0f,  0.0f, 1.f,     1.f, 0.f, 0.f,
			 0.0f,  0.0f,  0.0f, 1.f,     0.f, 1.f, 0.f,
			 0.0f,  0.2f,  0.0f, 1.f,     0.f, 1.f, 0.f,
			 0.0f,  0.0f,  0.0f, 1.f,     0.f, 0.f, 1.f,
			 0.0f,  0.0f,  0.2f, 1.f,     0.f, 0.f, 1.f
		};
		std::vector<unsigned int> cursorIndices =
		{
			0, 1,
			2, 3,
			4, 5
		};
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" },
			{ ShaderDataType::Float3, "color" }
		};
		m_Meshes[StaticMeshType::Cursor] = VertexArray::CreateWithBuffers(cursorVertices, cursorIndices, layout);
	}

	// Crosshair
	{
		std::vector<float> crosshairVertices =
		{
			-0.60f,  0.00f, 0.f, 1.f,
			-0.15f,  0.00f, 0.f, 1.f,
			 0.15f,  0.00f, 0.f, 1.f,
			 0.60f,  0.00f, 0.f, 1.f,
			 0.00f, -0.60f, 0.f, 1.f,
			 0.00f, -0.15f, 0.f, 1.f,
			 0.00f,  0.15f, 0.f, 1.f,
			 0.00f,  0.60f, 0.f, 1.f
		};
		std::vector<unsigned int> crosshairIndices =
		{
			0, 1,
			2, 3,
			4, 5,
			6, 7
		};
		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" }
		};
		m_Meshes[StaticMeshType::Crosshair] = VertexArray::CreateWithBuffers(crosshairVertices, crosshairIndices, layout);
	}

	// Grid
	{
		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		const int GRID_HALF_CELLS = 1000;
		const float GRID_EXTENT = static_cast<float>(GRID_HALF_CELLS);
		const float MINOR_LINE = 0.f;
		const float MAJOR_LINE = 1.f;
		const float ALONG_X = 0.f;
		const float ALONG_Z = 1.f;

		for (int i = -GRID_HALF_CELLS; i <= GRID_HALF_CELLS; i++)
		{
			const float coord = static_cast<float>(i);
			const float lineLevel = (i % 10 == 0) ? MAJOR_LINE : MINOR_LINE;

			vertices.insert(vertices.end(),
				{
					-GRID_EXTENT, 0.f, coord, 1.f, lineLevel, ALONG_X,
					 GRID_EXTENT, 0.f, coord, 1.f, lineLevel, ALONG_X,
					 coord, 0.f, -GRID_EXTENT, 1.f, lineLevel, ALONG_Z,
					 coord, 0.f,  GRID_EXTENT, 1.f, lineLevel, ALONG_Z,
				});

			for (int j = 0; j < 4; j++)
			{
				indices.push_back(static_cast<unsigned int>(indices.size()));
			}
		}

		BufferLayout layout =
		{
			{ ShaderDataType::Float4, "position" },
			{ ShaderDataType::Float2, "lineInfo" }
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
