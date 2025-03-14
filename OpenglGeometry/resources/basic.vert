#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
    gl_Position = position;
    out_Color = in_Color;
};