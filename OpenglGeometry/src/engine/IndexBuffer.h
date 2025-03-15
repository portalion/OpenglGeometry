#pragma once
#include "utils/GlCall.h"
#include <vector>

class IndexBuffer
{
private:
	size_t length;
public:
	IndexBuffer();
	IndexBuffer(std::vector<unsigned int> indices);
	~IndexBuffer();

	inline size_t GetLength() const { return length; }
	void AssignIndices(std::vector<unsigned int> indices);

	void Bind() const;
	void UnBind() const;
private:
	unsigned int id;
};

