#pragma once
#include "utils/GlCall.h"
#include "Algebra.h"

struct PositionVertexData
{
	Algebra::Vector4 Position;
};

struct PositionNormalVertexData
{
	Algebra::Vector4 Position;
	Algebra::Vector4  Normal;
};

struct PositionColorVertexData
{
	Algebra::Vector4 Position;
	Algebra::Vector4  Color;
};
