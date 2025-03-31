#version 460 core

layout(location = 0) out vec4 color;

uniform vec4 u_color = vec4(1.f, 0.2f, 0.f, 1.f);

void main()
{
   color =  u_color;
};