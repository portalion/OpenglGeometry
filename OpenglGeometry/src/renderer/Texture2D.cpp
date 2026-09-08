#include "Texture2D.h"
#include <GL/glew.h>

Texture2D::Texture2D(int width, int height)
	: m_Width(width), m_Height(height)
{
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture2D::Upload(const std::vector<uint8_t>& rgba)
{
	if (static_cast<int>(rgba.size()) < m_Width * m_Height * 4)
	{
		return;
	}
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind(int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}
