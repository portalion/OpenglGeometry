#version 430

layout (vertices=16) out;

uniform int u_subdivisions = 32;
uniform int v_subdivisions = 32;

in vec4 vPatchInfo[];
out vec4 tcPatchInfo[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcPatchInfo[gl_InvocationID] = vPatchInfo[gl_InvocationID];

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = ceil(v_subdivisions + 1);
        gl_TessLevelOuter[1] = ceil(u_subdivisions);
    }
}
