#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;

const float c_screenScale = 0.2;

void main()
{
    vec4 originView = g_viewMatrix * u_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    float distance = max(-originView.z, 0.0001);
    originView.xy += position.xy * distance * c_screenScale;
    gl_Position = g_projectionMatrix * originView;
};
