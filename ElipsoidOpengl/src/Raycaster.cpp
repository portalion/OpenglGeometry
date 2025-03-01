#include "Raycaster.h"
#include "utils/GlCall.h"

void Raycaster::SaveToBuffers()
{
	std::vector<RaycasterVertexData> vertices = {
		RaycasterVertexData
		{
			.positions = { -0.5f, -0.5f, 0.0f, 1.f },
			.colors = { 0.5f, 0.5f, 0.5f, 1.f }
		}, 
		RaycasterVertexData
		{
			.positions = { 0.5f, -0.5f, 0.0f, 1.f },
			.colors = { 0.5f, 0.5f, 0.5f, 1.f }
		},
		RaycasterVertexData
		{
			.positions = { 0.0f,  0.5f, 0.0f, 1.f },
			.colors = { 0.5f, 0.5f, 0.5f, 1.f }
		}
	};

	GLCall(glBindVertexArray(VAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 
		vertices.size() * sizeof(RaycasterVertexData), vertices.data(), GL_DYNAMIC_DRAW));
	GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RaycasterVertexData), 
		(void*)offsetof(RaycasterVertexData, positions)));
	GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RaycasterVertexData), 
		(void*)offsetof(RaycasterVertexData, colors)));
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

void Raycaster::RenderResult()
{
	this->SaveToBuffers();

	usedShader.bind();
	GLCall(glBindVertexArray(VAO));
	GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	GLCall(glBindVertexArray(0));
	usedShader.unBind();
}
