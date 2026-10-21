#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Texel;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
uniform sampler2D u_heightMap;
uniform float u_heightScale = 1.0;

out vec3 v_localPos;
out vec3 v_localNormal;
out vec3 v_worldPos;

float HeightAt(ivec2 texel, ivec2 size)
{
    return texelFetch(u_heightMap, clamp(texel, ivec2(0), size - 1), 0).r * u_heightScale;
}

void main()
{
    vec2 uv = a_Position.xy;
    bool isTop = a_Position.z > 0.5;
    ivec2 size = textureSize(u_heightMap, 0);
    ivec2 texel = ivec2(a_Texel);

    float height = isTop ? HeightAt(texel, size) : 0.0;

    // material X -> X, material Y -> -Z, height -> Y
    vec3 local = vec3(uv.x - 0.5, height, 0.5 - uv.y);

    // Smooth height-map normal in local space (only meaningful for top vertices)
    vec2 cell = 1.0 / vec2(max(size - 1, ivec2(1)));
    float dhdu = (HeightAt(texel + ivec2(1, 0), size) - HeightAt(texel - ivec2(1, 0), size)) / (2.0 * cell.x);
    float dhdv = (HeightAt(texel + ivec2(0, 1), size) - HeightAt(texel - ivec2(0, 1), size)) / (2.0 * cell.y);
    v_localNormal = normalize(vec3(-dhdu, 1.0, dhdv));

    vec4 world = u_modelMatrix * vec4(local, 1.0);
    v_localPos = local;
    v_worldPos = world.xyz;
    gl_Position = g_projectionMatrix * g_viewMatrix * world;
}