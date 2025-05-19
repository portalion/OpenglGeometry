#version 430

layout (vertices=16) out;

uniform int u_subdivisions = 32;
uniform int v_subdivisions = 32;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        // Outer tessellation levels: 4 edges
        gl_TessLevelOuter[0] = ceil(v_subdivisions + 1);
        gl_TessLevelOuter[1] = ceil(u_subdivisions);

    }
}
