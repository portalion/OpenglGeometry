#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

void main()
{
    gl_Position = g_projectionMatrix * g_viewMatrix * u_modelMatrix * position;
};