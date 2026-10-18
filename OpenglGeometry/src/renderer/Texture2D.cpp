#include "Texture2D.h"
#include <GL/glew.h>

namespace
{
	struct GLFormat
	{
		GLint internalFormat;
		GLenum format;
		GLenum type;
		GLint filter;
	};

	GLFormat ToGL(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R32F:
			return { GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR };
		case TextureFormat::RGBA8:
		default:
			return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST };
		}
	}
}

Texture2D::Texture2D(int width, int height, TextureFormat format)
	: m_Width(width), m_Height(height), m_Format(format)
{
	const GLFormat gl = ToGL(m_Format);

	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	Allocate();
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture2D::Allocate()
{
	const GLFormat gl = ToGL(m_Format);

	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, gl.internalFormat, m_Width, m_Height, 0, gl.format, gl.type, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Resize(int width, int height)
{
	if (width == m_Width && height == m_Height)
	{
		return;
	}
	m_Width = width;
	m_Height = height;
	Allocate();
}

void Texture2D::Upload(const std::vector<uint8_t>& rgba)
{
	if (m_Format != TextureFormat::RGBA8 || static_cast<int>(rgba.size()) < m_Width * m_Height * 4)
	{
		return;
	}
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Upload(const std::vector<float>& values)
{
	if (m_Format != TextureFormat::R32F || static_cast<int>(values.size()) < m_Width * m_Height)
	{
		return;
	}
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RED, GL_FLOAT, values.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Upload(const float& value)
{
	if (m_Format != TextureFormat::R32F)
	{
		return;
	}
	glClearTexImage(m_RendererID, 0, GL_RED, GL_FLOAT, &value);
}

void Texture2D::Bind(int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}
