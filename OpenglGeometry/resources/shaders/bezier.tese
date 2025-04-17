#version 460 core

layout(isolines, equal_spacing, ccw) in;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

vec4 Bezier(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float t)
{
    float u = 1.0 - t;
    float b3 = t * t * t;
    float b2 = 3.0 * t * t * u;
    float b1 = 3.0 * t * u * u;
    float b0 = u * u * u;

    vec4 result = b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;
    result.w = 1.0;

    return result;
}

void main()
{
    float t = gl_TessCoord.x;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    gl_Position = u_projectionMatrix * u_viewMatrix * Bezier(p0, p1, p2, p3, t);
}
