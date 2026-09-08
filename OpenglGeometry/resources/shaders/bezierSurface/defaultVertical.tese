#version 430

layout (isolines, equal_spacing, ccw) in;

uniform mat4 g_viewMatrix;
uniform mat4 g_projectionMatrix;
uniform int v_subdivisions;

in vec4 tcPatchInfo[];
out vec2 v_uv;

vec3 interpolateRow(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float u) {
    float u1 = 1.0 - u;
    return u1*u1*u1*p0 +
           3.0*u*u1*u1*p1 +
           3.0*u*u*u1*p2 +
           u*u*u*p3;
}

void main() {
    float u = gl_TessCoord.y;
    float v = gl_TessCoord.x;

    float uSub = v_subdivisions;
    u = (uSub + 1) * u / uSub;

    vec3 row0 = interpolateRow(gl_in[0].gl_Position.xyz,  gl_in[1].gl_Position.xyz,
                               gl_in[2].gl_Position.xyz,  gl_in[3].gl_Position.xyz,  u);
    vec3 row1 = interpolateRow(gl_in[4].gl_Position.xyz,  gl_in[5].gl_Position.xyz,
                               gl_in[6].gl_Position.xyz,  gl_in[7].gl_Position.xyz,  u);
    vec3 row2 = interpolateRow(gl_in[8].gl_Position.xyz,  gl_in[9].gl_Position.xyz,
                               gl_in[10].gl_Position.xyz, gl_in[11].gl_Position.xyz, u);
    vec3 row3 = interpolateRow(gl_in[12].gl_Position.xyz, gl_in[13].gl_Position.xyz,
                               gl_in[14].gl_Position.xyz, gl_in[15].gl_Position.xyz, u);

    vec3 finalPos = interpolateRow(row0, row1, row2, row3, v);

    vec4 pInfo = tcPatchInfo[0];   // (patchCol, patchRow, patchCols, patchRows)
    float gu = (pInfo.x + clamp(u, 0.0, 1.0)) / max(pInfo.z, 1.0);
    float gv = (pInfo.y + clamp(v, 0.0, 1.0)) / max(pInfo.w, 1.0);
    v_uv = vec2(gu, gv);

    gl_Position = g_projectionMatrix * g_viewMatrix * vec4(finalPos, 1.0);
}
