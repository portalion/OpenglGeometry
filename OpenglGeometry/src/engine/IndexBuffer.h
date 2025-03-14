#pragma once
#include "utils/GlCall.h"
#include <vector>

class IndexBuffer
{
public:
	IndexBuffer();
	IndexBuffer(std::vector<unsigned int> indices);
	~IndexBuffer();

	void AssignIndices(std::vector<unsigned int> indices) const;

	void Bind() const;
	void UnBind() const;
private:
	unsigned int id;
};

