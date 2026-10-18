#pragma once
#include <cstdint>
#include <vector>

enum class TextureFormat
{
	RGBA8,	// 4 x uint8, nearest filtering — trim masks
	R32F	// 1 x float, linear filtering — height maps
};

// A single-image 2D texture whose storage can be reallocated with Resize.
class Texture2D
{
public:
	Texture2D(int width, int height, TextureFormat format = TextureFormat::RGBA8);
	~Texture2D();

	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

	// Reallocates storage; contents are undefined until the next Upload.
	void Resize(int width, int height);

	void Upload(const std::vector<uint8_t>& rgba);
	void Upload(const std::vector<float>& values);
	void Upload(const float& value);
	void Bind(int unit) const;

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }
	TextureFormat Format() const { return m_Format; }

private:
	void Allocate();

	uint32_t m_RendererID = 0;
	int m_Width = 0;
	int m_Height = 0;
	TextureFormat m_Format = TextureFormat::RGBA8;
};
