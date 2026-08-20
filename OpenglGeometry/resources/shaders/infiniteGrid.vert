#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 lineInfo;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_worldPos;
layout(location = 2) out vec3 out_eyePos;

uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
const float c_minHeight = 0.001;

const vec3 c_minorColor = vec3(0.30);
const vec3 c_majorColor = vec3(0.55);
const vec3 c_xAxisColor = vec3(0.85, 0.25, 0.25);
const vec3 c_zAxisColor = vec3(0.25, 0.45, 0.90);

void main()
{
    vec3 eye = (inverse(g_viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    float height = max(abs(eye.y), c_minHeight);
    float level = log(height * c_cellsPerHeight) / log(10.0);
    float decadeFraction = fract(level);
    float cellSize = pow(10.0, floor(level));

    float snapPeriod = cellSize * 10.0;
    vec3 world = vec3(position.x * cellSize, 0.0, position.z * cellSize);
    world.xz += round(eye.xz / snapPeriod) * snapPeriod;

    float isMajor = lineInfo.x;
    vec3 color = mix(c_minorColor, mix(c_majorColor, c_minorColor, decadeFraction), isMajor);
    float alpha = mix(1.0 - decadeFraction, 1.0, isMajor);

    float lineCoord = mix(world.z, world.x, lineInfo.y);
    if (abs(lineCoord) < 0.5 * cellSize)
    {
        color = mix(c_xAxisColor, c_zAxisColor, lineInfo.y);
        alpha = 1.0;
    }

    out_color = vec4(color, alpha);
    out_worldPos = world;
    out_eyePos = eye;

    gl_Position = g_projectionMatrix * g_viewMatrix * vec4(world, 1.0);
};
