#version 460 core

layout(isolines, equal_spacing, ccw) in;

uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
uniform int v_subdivisions;

vec3 getPoint(int i) { return gl_in[i].gl_Position.xyz; }

vec3 cubic(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float s = 1.0 - t;
    return s*s*s*p0 + 3.0*s*s*t*p1 + 3.0*s*t*t*p2 + t*t*t*p3;
}

const int P00 = 0, P01 = 3, P10 = 16, P11 = 19;
const int VuvAt00 = 6, VvuAt00 = 5;
const int VuvAt01 = 7, VvuAt01 = 8;
const int VuvAt10 = 12, VvuAt10 = 11;
const int VuvAt11 = 13, VvuAt11 = 14;

vec3 evaluate(float u, float v) {
    vec3 v_ab00 = (u != 0.0 && v != 0.0) ? (u * getPoint(VuvAt00) + v * getPoint(VvuAt00)) / (u + v)
                                         : 0.5 * (getPoint(VuvAt00) + getPoint(VvuAt00));
    vec3 v_ab01 = (u != 1.0 && v != 0.0) ? ((1.0 - u) * getPoint(VuvAt01) + v * getPoint(VvuAt01)) / (1.0 - u + v)
                                         : 0.5 * (getPoint(VuvAt01) + getPoint(VvuAt01));
    vec3 v_ab10 = (u != 0.0 && v != 1.0) ? (u * getPoint(VuvAt10) + (1.0 - v) * getPoint(VvuAt10)) / (1.0 + u - v)
                                         : 0.5 * (getPoint(VvuAt10) + getPoint(VuvAt10));
    vec3 v_ab11 = (u != 1.0 && v != 1.0) ? ((1.0 - u) * getPoint(VuvAt11) + (1.0 - v) * getPoint(VvuAt11)) / (2.0 - u - v)
                                         : 0.5 * (getPoint(VuvAt11) + getPoint(VvuAt11));

    vec3 row0 = cubic(getPoint(P00), getPoint(1),  getPoint(2),  getPoint(P01), v);
    vec3 row1 = cubic(getPoint(4),   v_ab00, v_ab01, getPoint(9),   v);
    vec3 row2 = cubic(getPoint(10),  v_ab10, v_ab11, getPoint(15),  v);
    vec3 row3 = cubic(getPoint(P10), getPoint(17), getPoint(18), getPoint(P11), v);

    return cubic(row0, row1, row2, row3, u);
}

void main() {
    float u = gl_TessCoord.y;
    float v = gl_TessCoord.x;

    float vSub = v_subdivisions;
    u = (vSub + 1) * u / vSub;

    vec3 pos = evaluate(u, v);
    gl_Position = g_projectionMatrix * g_viewMatrix * vec4(pos, 1.0);
}
