#include "Raycaster.h"
#include "utils/GlCall.h"
#include "RaycastableEllipsoid.h"


void Raycaster::RunRays(Window* window, int thickness)
{
	RaycastableEllipsoid ellispoid;
	textureData.clear();
	const float width = static_cast<float>(window->GetWidth() / 100.f);
	const float height = static_cast<float>(window->GetHeight() / 100.f);
	float thicknessf = static_cast<float>(thickness) / 100.f;
	float thicknessStep = thicknessf / static_cast<float>(thickness);

	for (float x = -width / 2.f; x < width / 2.f; x += thicknessf)
	{
		for (float y = -height / 2.f; y < height / 2.f; y += thicknessf)
		{
			auto foundZ = ellispoid.FindZ(x, y);
			if (foundZ.first)
			{
				float lightIntensity = GetLightIntensity(
					Algebra::Vector4{ 10.f, 10.f, 10.f, 0.f },
					Algebra::Vector4{ x, y, foundZ.second, 0.f },
					ellispoid.FindGradient(x, y, foundZ.second).Normalize(),
					6);

				for (float i = 0; i < thicknessf; i+= thicknessStep)
				{
					for (float j = 0; j < thicknessf; j+= thicknessStep)
					{
						textureData.push_back(RaycasterVertexData{
						.position{(x + i) / width * 2.f, (y + j) / height * 2.f, 0, 1},
						.color{ lightIntensity, lightIntensity,  lightIntensity, 1.f},
							});
					}
				}
			}
		}
	}
}

float Raycaster::GetLightIntensity(Algebra::Vector4 observatorPosition, Algebra::Vector4 point, Algebra::Vector4 gradient, int m)
{
	Algebra::Vector4 v = (observatorPosition - point).Normalize();

	float intensity = std::max(v * gradient, 0.f);
	return std::powf(intensity, m);
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
	this->RunRays(window, 16);
	this->SaveToBuffers();

	usedShader.bind();
	GLCall(glBindVertexArray(VAO));
	GLCall(glDrawArrays(GL_POINTS, 0, textureData.size()));
	GLCall(glBindVertexArray(0));
	usedShader.unBind();
}
