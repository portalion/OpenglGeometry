#include "Raycaster.h"
#include "utils/GlCall.h"

void Raycaster::SaveToBuffers()
{
	float vertices[] = {
	-0.5f, -0.5f, 0.0f, 1.f,
	 0.5f, -0.5f, 0.0f, 1.f,
	 0.0f,  0.5f, 0.0f, 1.f
	};

	GLCall(glBindVertexArray(VAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
	GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr));
	GLCall(glEnableVertexAttribArray(0));
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
