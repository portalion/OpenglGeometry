#include "Raycaster.h"
#include "utils/GlCall.h"
#include "RaycastableEllipsoid.h"


void Raycaster::RunRays(Window* window)
{
	textureData.clear();
	const float width = static_cast<float>(window->GetWidth() / 100.f);
	const float height = static_cast<float>(window->GetHeight() / 100.f);
	float thicknessf = static_cast<float>(rayThickness) / 100.f;
	float thicknessStep = thicknessf / static_cast<float>(rayThickness);

	for (float x = -width / 2.f; x < width / 2.f; x += thicknessf)
	{
		for (float y = -height / 2.f; y < height / 2.f; y += thicknessf)
		{
			auto foundZ = shape.FindZ(x, y);
			if (foundZ.first)
			{
				float lightIntensity = GetLightIntensity(
					Algebra::Vector4{ 10.f, 10.f, 10.f, 0.f },
					Algebra::Vector4{ x, y, foundZ.second, 0.f },
					shape.FindGradient(x, y, foundZ.second).Normalize());

				for (float i = -thicknessf/2.f; i < thicknessf/2.f; i+= thicknessStep)
				{
					for (float j = -thicknessf / 2.f; j < thicknessf /2.f; j+= thicknessStep)
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

float Raycaster::GetLightIntensity(Algebra::Vector4 observatorPosition, Algebra::Vector4 point, Algebra::Vector4 gradient)
{
	Algebra::Vector4 v = (observatorPosition - point).Normalize();

	float intensity = std::max(v * gradient, 0.f);
	return std::powf(intensity, static_cast<float>(intensityPower));
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
	intensityPower = 5;
	rayThickness = 1;
	shouldRecalculate = true;

	GLCall(glGenBuffers(1, &VBO));
	GLCall(glGenVertexArrays(1, &VAO));
}

void Raycaster::RayCast(Window* window)
{
	if (!shouldRecalculate)
	{
		return;
	}
	this->RunRays(window);
	this->SaveToBuffers();

	shouldRecalculate = false;
}

void Raycaster::RenderMenu()
{
	shouldRecalculate |= shape.RenderMenu();
	if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_Leaf))
	{
		shouldRecalculate |= ImGui::InputInt("m", &intensityPower, 1, 10);
		shouldRecalculate |= ImGui::SliderInt("accuracy", &rayThickness, 1, 32);
	}
}

void Raycaster::RenderResult()
{
	usedShader.bind();
	GLCall(glBindVertexArray(VAO));
	GLCall(glDrawArrays(GL_POINTS, 0, textureData.size()));
	GLCall(glBindVertexArray(0));
	usedShader.unBind();
}
