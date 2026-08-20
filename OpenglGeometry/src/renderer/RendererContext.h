#pragma once
#include <Algebra.h>
#include <unordered_map>
#include "core/Globals.h"

struct SceneContext
{
	Algebra::Matrix4 ViewMatrix;
	Algebra::Matrix4 ProjectionMatrix;
	Algebra::Vector4 CameraPosition;
	Algebra::Vector4 CameraTarget;
};

struct EntityContext
{
	Algebra::Matrix4 Position = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Rotation = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Scale = Algebra::Matrix4::Identity();
	Algebra::Vector4 Color = Globals::defaultPointsColor;
	int SamplesU = 32;
	int SamplesV = 32;
};

struct UniformContext
{
	std::unordered_map<std::string, Algebra::Vector4> Vector4Uniforms;
	std::unordered_map<std::string, Algebra::Matrix4> Matrix4Uniforms;
	std::unordered_map<std::string, int> IntUniforms;
};