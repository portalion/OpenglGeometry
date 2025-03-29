#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

void main()
{
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * position;
};