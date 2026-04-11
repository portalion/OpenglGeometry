#pragma once
#include <Algebra.h>

struct RendererContext
{
	Algebra::Matrix4 Position = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Rotation = Algebra::Matrix4::Identity();
	Algebra::Matrix4 Scale = Algebra::Matrix4::Identity();
	Algebra::Matrix4 u_ProjectionMatrix = Algebra::Matrix4::Identity();
	Algebra::Matrix4 u_ViewMatrix = Algebra::Matrix4::Identity();
};