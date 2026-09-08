#pragma once
#include <cstdint>
#include <vector>

// A single-image RGBA8 texture. The project's first — used for intersection trim masks.
class Texture2D
{
public:
	Texture2D(int width, int height);
	~Texture2D();

	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

	void Upload(const std::vector<uint8_t>& rgba);
	void Bind(int unit) const;

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }

private:
	uint32_t m_RendererID = 0;
	int m_Width = 0;
	int m_Height = 0;
};
