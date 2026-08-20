#version 460 core

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_worldPos;
layout(location = 2) in vec3 in_eyePos;
layout(location = 3) in vec3 in_gridInfo;

layout(location = 0) out vec4 out_color;

const float c_fadeStartScales = 2.0;
const float c_fadeEndScales = 6.0;

const float c_grazingFullSlope = 0.12;
const float c_grazingFloor = 0.2;

void main()
{
    vec2 focus = in_gridInfo.xy;
    float viewScale = in_gridInfo.z;

    float distanceToFocus = length(in_worldPos.xz - focus);
    float distanceFade = 1.0 - smoothstep(
        c_fadeStartScales * viewScale, c_fadeEndScales * viewScale, distanceToFocus);

    float slope = abs(normalize(in_worldPos - in_eyePos).y);
    float grazingFade = mix(c_grazingFloor, 1.0, smoothstep(0.0, c_grazingFullSlope, slope));

    float alpha = in_color.a * distanceFade * grazingFade;
    if (alpha <= 0.002)
    {
        discard;
    }

    out_color = vec4(in_color.rgb, alpha);
};
