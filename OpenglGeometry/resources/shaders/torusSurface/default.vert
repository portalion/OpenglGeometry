#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 a_uv;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

out vec2 v_uv;

void main()
{
    v_uv = a_uv.xy;
    gl_Position = g_projectionMatrix * g_viewMatrix * u_modelMatrix * position;
}
