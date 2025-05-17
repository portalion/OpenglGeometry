#version 430

layout (vertices=16) out;

uniform uint u_subdivisions = 32;
uniform uint v_subdivisions = 32;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        // Outer tessellation levels: 4 edges
        gl_TessLevelOuter[0] = 64.0;
        gl_TessLevelOuter[1] = 64.0;
        gl_TessLevelOuter[2] = 64.0;
        gl_TessLevelOuter[3] = 64.0;

    }
}
