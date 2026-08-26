#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

out vec3 v_color;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

const float c_screenScale = 0.2;

void main()
{
    vec4 originView = g_viewMatrix * u_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);

    vec3 axisView = mat3(g_viewMatrix * u_modelMatrix) * position.xyz;
    float distance = max(-originView.z, 0.0001);

    vec4 viewPos = originView + vec4(axisView * distance * c_screenScale, 0.0);
    gl_Position = g_projectionMatrix * viewPos;

    v_color = color;
};
