#include "Raycaster.h"
#include "utils/GlCall.h"
#include "RaycastableEllipsoid.h"


void Raycaster::RunRays(Window* window, int xSize, int ySize)
{
	RaycastableEllipsoid ellispoid;
	textureData.clear();
	const float width = static_cast<float>(window->GetWidth() / 100.f);
	const float height = static_cast<float>(window->GetHeight() / 100.f);
	const float xStep = width / xSize;
	const float yStep = height / ySize;
	for (float x = -width / 2.f; x < width / 2.f; x += xStep)
	{
		for (float y = -height / 2.f; y < height / 2.f; y += yStep)
		{
			auto foundZ = ellispoid.FindZ(x, y);
			if (foundZ.first)
			{
				textureData.push_back(RaycasterVertexData{
				   .position{x / width * 2.f, y / height * 2.f, 0, 1},
				   .color{0.5f, 0.5f, 0.5f, 1.f},
					});
			}
		}
	}
}

void Raycaster::SaveToBuffers()
{
	GLCall(glBindVertexArray(VAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 
		textureData.size() * sizeof(RaycasterVertexData), textureData.data(), GL_DYNAMIC_DRAW));
	GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RaycasterVertexData), 
		(void*)offsetof(RaycasterVertexData, position)));
	GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RaycasterVertexData), 
		(void*)offsetof(RaycasterVertexData, color)));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glEnableVertexAttribArray(1));
	GLCall(glBindVertexArray(0));
}

Raycaster::Raycaster()
	:usedShader{ "resources/temp.shader" }, VBO{ 0 }, VAO{ 0 }
{
	GLCall(glGenBuffers(1, &VBO));
	GLCall(glGenVertexArrays(1, &VAO));
	
}

void Raycaster::RenderResult(Window* window)
{
	this->RunRays(window, window->GetWidth(), window->GetHeight());
	this->SaveToBuffers();

	usedShader.bind();
	GLCall(glBindVertexArray(VAO));
	GLCall(glDrawArrays(GL_POINTS, 0, textureData.size()));
	GLCall(glBindVertexArray(0));
	usedShader.unBind();
}
