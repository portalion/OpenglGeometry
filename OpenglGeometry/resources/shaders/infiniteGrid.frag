#version 460 core

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_worldPos;
layout(location = 2) in vec3 in_eyePos;

layout(location = 0) out vec4 out_color;

const float c_fadeStartHeights = 3.0;
const float c_fadeEndHeights = 8.0;

void main()
{
    float height = max(abs(in_eyePos.y), 1e-4);
    float distanceToCamera = length(in_worldPos.xz - in_eyePos.xz);

    float fade = 1.0 - smoothstep(
        c_fadeStartHeights * height, c_fadeEndHeights * height, distanceToCamera);

    float alpha = in_color.a * fade;
    if (alpha <= 0.002)
    {
        discard;
    }

    out_color = vec4(in_color.rgb, alpha);
};
