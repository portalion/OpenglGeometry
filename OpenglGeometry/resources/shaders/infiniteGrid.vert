#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec4 out_worldPos;
layout(location = 2) out float scalling;
layout(location = 3) out float fraction;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
uniform vec4 g_cameraPosition;

void main()
{
    out_color = color;
    out_worldPos = u_modelMatrix * position;
    float dist = max(abs(g_cameraPosition.z), 1e-6);
    float log10dist = log(dist) / log(10.0);
    float decade = floor(log10dist);
    float stepMul = pow(10.0, decade);

    scalling = stepMul * 10;
    fraction = log10dist - decade;

    out_worldPos.xyz *= scalling;

    vec2 movement = floor(g_cameraPosition.xy / 10);

    out_worldPos.xy -= movement * 10;

    gl_Position = g_projectionMatrix * g_viewMatrix * out_worldPos;
};