#version 460 core

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec4 in_worldPos;

layout(location = 0) out vec4 out_color;
uniform vec4 uCameraPos;
void main()
{
   out_color = vec4(in_color, 1.0);
};