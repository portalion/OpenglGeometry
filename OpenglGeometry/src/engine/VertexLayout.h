#pragma once
#include "Algebra.h"
#include "utils/GlCall.h"

class VertexLayout
{
public:
	void virtual EnableAttribArrays() const = 0;
};

struct PositionVertexLayout : VertexLayout
{
	Algebra::Vector4 Position;

	void EnableAttribArrays() const override
	{
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionVertexLayout), 
			(void*)offsetof(PositionVertexLayout, Position));
		glEnableVertexAttribArray(0);
	}
};

struct PositionColorVertexLayout : VertexLayout
{
	Algebra::Vector4 Position;
	Algebra::Vector4 Color;

	void EnableAttribArrays() const override
	{
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionColorVertexLayout),
			(void*)offsetof(PositionColorVertexLayout, Position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PositionColorVertexLayout), 
			(void*)offsetof(PositionColorVertexLayout, Color));
		glEnableVertexAttribArray(1);
	}
};

struct PositionNormalVertexLayout : VertexLayout
{
	Algebra::Vector4 Position;
	Algebra::Vector4 Normal;

	void EnableAttribArrays() const override
	{
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionNormalVertexLayout),
			(void*)offsetof(PositionNormalVertexLayout, Position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PositionNormalVertexLayout),
			(void*)offsetof(PositionNormalVertexLayout, Normal));
		glEnableVertexAttribArray(1);
	}
};