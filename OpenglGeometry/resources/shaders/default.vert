#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main()
{
    gl_Position = u_projectionMatrix * u_viewMatrix * position;
};