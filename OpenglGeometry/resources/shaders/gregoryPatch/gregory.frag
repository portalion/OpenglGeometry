#version 460 core

out vec4 fragColor;

uniform vec4 u_color = vec4(0.2, 0.55, 1.0, 1.0);

void main()
{
    fragColor = u_color;
}
