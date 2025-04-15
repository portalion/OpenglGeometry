#version 460 core

layout(isolines, equal_spacing, ccw) in;

layout(std430, binding = 0) buffer ControlPoints {
    vec4 controlPoints[]; // dostêpna globalnie
};

uniform int pointCount;

float binomial(int n, int k)
{
    float res = 1.0;
    for (int i = 1; i <= k; ++i)
        res *= float(n - (k - i)) / float(i);
    return res;
}

vec4 bezierPoint(float t)
{
    vec4 result = vec4(0.0);
    float u = 1.0 - t;

    for (int i = 0; i < pointCount; ++i)
    {
        float bin = binomial(pointCount - 1, i);
        float coeff = bin * pow(u, pointCount - 1 - i) * pow(t, i);
        result += coeff * controlPoints[i];
    }

    return result;
}

void main()
{
    float t = gl_TessCoord.x;

    gl_Position = bezierPoint(t);
}
