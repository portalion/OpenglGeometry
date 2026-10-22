#include "StorageBuffer.h"
#include <GL/glew.h>

StorageBuffer::StorageBuffer()
{
	glCreateBuffers(1, &m_RendererID);
}

StorageBuffer::~StorageBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}

void StorageBuffer::SetData(const void* data, uint32_t size)
{
	glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
	m_Size = size;
}

void StorageBuffer::BindBase(uint32_t binding) const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
}
