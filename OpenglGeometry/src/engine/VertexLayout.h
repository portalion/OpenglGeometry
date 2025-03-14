#pragma once
#include "Algebra.h"
#include "utils/GlCall.h"

enum class VertexDataType
{
	PositionVertexData = 1,
	PositionColorVertexData = 2,
	PositionNormalVertexData = 3,
};

struct PositionVertexData
{
	Algebra::Vector4 Position;
};

struct PositionNormalVertexData
{
	Algebra::Vector4 Position;
	Algebra::Vector4 Normal;
};

struct PositionColorVertexData
{
	Algebra::Vector4 Position;
	Algebra::Vector4 Color;
};