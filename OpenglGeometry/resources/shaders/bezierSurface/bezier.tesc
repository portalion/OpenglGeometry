#version 460 core

layout (vertices=16) out;

uniform int u_subdivisions = 32;
uniform int v_subdivisions = 32;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelOuter[0] = ceil(u_subdivisions);
    gl_TessLevelOuter[1] = ceil(v_subdivisions);
}
