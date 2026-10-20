#version 460 core

// (u, v, isTop, normalCode), see MaterialBlock.cpp
layout(location = 0) in vec4 a_Position;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
uniform sampler2D u_heightMap;

out vec3 v_worldPos;
out vec3 v_normal;

float HeightAt(ivec2 texel, ivec2 size)
{
    return texelFetch(u_heightMap, clamp(texel, ivec2(0), size - 1), 0).r;
}

vec3 SideNormal(int code)
{
    switch (code)
    {
    case 1: return vec3(-1.0, 0.0, 0.0);
    case 2: return vec3(1.0, 0.0, 0.0);
    case 3: return vec3(0.0, 0.0, 1.0);
    case 4: return vec3(0.0, 0.0, -1.0);
    default: return vec3(0.0, -1.0, 0.0);
    }
}

void main()
{
    vec2 uv = a_Position.xy;
    bool isTop = a_Position.z > 0.5;
    int normalCode = int(round(a_Position.w));

    ivec2 size = textureSize(u_heightMap, 0);
    ivec2 texel = ivec2(round(uv * vec2(size - 1)));
    float height = isTop ? HeightAt(texel, size) : 0.0;

    // material X -> world X, material Y -> world -Z, height -> world Y
    vec3 local = vec3(uv.x - 0.5, height, 0.5 - uv.y);
    vec4 world = u_modelMatrix * vec4(local, 1.0);

    if (normalCode == 0)
    {
        vec2 extent = vec2(length(u_modelMatrix[0].xyz), length(u_modelMatrix[2].xyz));
        vec2 cell = extent / vec2(max(size - 1, ivec2(1)));

        float dhdx = (HeightAt(texel + ivec2(1, 0), size) - HeightAt(texel - ivec2(1, 0), size)) / (2.0 * cell.x);
        float dhdy = (HeightAt(texel + ivec2(0, 1), size) - HeightAt(texel - ivec2(0, 1), size)) / (2.0 * cell.y);
        // world Z runs opposite to material Y
        v_normal = normalize(vec3(-dhdx, 1.0, dhdy));
    }
    else
    {
        v_normal = SideNormal(normalCode);
    }

    v_worldPos = world.xyz;
    gl_Position = g_projectionMatrix * g_viewMatrix * world;
}
