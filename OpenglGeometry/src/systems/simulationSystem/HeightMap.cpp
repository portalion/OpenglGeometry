#include "HeightMap.h"
#include <cmath>
#include "managers/ShaderManager.h"

void HeightMap::Reset()
{
	m_HeightMap->Upload(m_CurrentHeight);
}

HeightMap::HeightMap()
{
	m_HeightMap = CreateRef<Texture2D>(0, 0, TextureFormat::R32F);
}

void HeightMap::ChangeSize(unsigned int width, unsigned int height, float value)
{
	if (m_CurrentHeight == value && m_HeightMap->Width() == width && m_HeightMap->Height() == height)
	{
		return;
	}

	m_CurrentHeight = value;
	m_HeightMap->Resize(width, height);
	m_HeightMap->Upload(value);
}

void HeightMap::Mill(const Algebra::Vector4& tool, bool isFlat, float sizeX, float sizeY)
{
	const float width = static_cast<float>(Width());
	const float height = static_cast<float>(Height());

	// Only dispatch over the texels the tool can touch.
	const int minX = static_cast<int>(std::floor((tool.x - tool.w + 0.5f * sizeX) / sizeX * width));
	const int minY = static_cast<int>(std::floor((tool.y - tool.w + 0.5f * sizeY) / sizeY * height));
	const int maxX = static_cast<int>(std::ceil((tool.x + tool.w + 0.5f * sizeX) / sizeX * width));
	const int maxY = static_cast<int>(std::ceil((tool.y + tool.w + 0.5f * sizeY) / sizeY * height));
	if (maxX < 0 || maxY < 0 || minX >= width || minY >= height)
	{
		return;
	}

	auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::HeightMapMill);

	UniformContext context;
	context.TextureUniforms["uHeightMap"] = m_HeightMap;
	context.Vector4Uniforms["uTool"] = tool;
	context.Vector4Uniforms["uMaterial"] = Algebra::Vector4(sizeX, sizeY, 0.f, 0.f);
	context.Vector4Uniforms["uRegion"] = Algebra::Vector4(static_cast<float>(minX), static_cast<float>(minY), 0.f, 0.f);
	context.IntUniforms["uIsFlat"] = isFlat ? 1 : 0;

	shader->Bind();
	shader->ApplyContext(context);
	shader->Dispatch((maxX - minX + 16) / 16, (maxY - minY + 16) / 16);
	shader->UnBind();
}
