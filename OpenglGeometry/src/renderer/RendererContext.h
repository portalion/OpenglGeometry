#pragma once
#include <Algebra.h>
#include <unordered_map>

struct SceneContext
{
	Algebra::Matrix4 ViewMatrix;
	Algebra::Matrix4 ProjectionMatrix;
	Algebra::Vector4 CameraPosition;
};

struct EntityContext
{
	Algebra::Matrix4 Position = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Rotation = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Scale = Algebra::Matrix4::Identity();
};

struct UniformContext
{
	std::unordered_map<std::string, Algebra::Vector4> Vector4Uniforms;
	std::unordered_map<std::string, Algebra::Matrix4> Matrix4Uniforms;
};