#version 460 core

layout(location = 0) in vec4 in_Color;
layout(location = 0) out vec4 color;

void main()
{
   color = in_Color + vec4(0.2f, 0.2f, 0.f, 0.f);
};