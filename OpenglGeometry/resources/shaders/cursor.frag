#version 460 core

layout(location = 0) out vec4 color;

in vec3 v_color;

uniform vec4 u_color = vec4(1.0);

void main()
{
   color = vec4(v_color, 1.0) * u_color;
};
