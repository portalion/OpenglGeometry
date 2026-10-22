#pragma once
#include <cstdint>

// Shader storage buffer, read in shaders as `layout(std430, binding = N) buffer`.
class StorageBuffer
{
public:
	StorageBuffer();
	~StorageBuffer();

	StorageBuffer(const StorageBuffer&) = delete;
	StorageBuffer& operator=(const StorageBuffer&) = delete;

	void SetData(const void* data, uint32_t size);
	void BindBase(uint32_t binding) const;

	uint32_t Size() const { return m_Size; }
private:
	uint32_t m_RendererID = 0;
	uint32_t m_Size = 0;
};
