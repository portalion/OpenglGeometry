#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec4 out_worldPos;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform vec4 uCameraPos;
void main()
{
    out_color = color;
    out_worldPos = u_modelMatrix * position;
    float dist = max(abs(uCameraPos.z), 1e-6);

    float decade = floor(log(dist) / log(10.0));

    float stepMul = pow(10.0, decade);

    out_worldPos.xyz *= stepMul * 50;

    vec2 movement = floor(uCameraPos.xy / 10);

    out_worldPos.xy -= movement * 10;

    gl_Position = u_projectionMatrix * u_viewMatrix * out_worldPos;
};