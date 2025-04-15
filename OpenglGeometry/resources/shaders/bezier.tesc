#version 460 core

layout(vertices = 4) out; 

void main()
{
    // Pass control points to evaluation shader
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Set tessellation levels (number of segments)
    if (gl_InvocationID == 0)
    {
        // Outer levels = curve resolution
        gl_TessLevelOuter[0] = 10.0; // Number of curve segments
        gl_TessLevelOuter[1] = 10.0;  // Not used for isolines
    }
}
