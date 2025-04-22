#version 460 core

layout(vertices = 4) out;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform vec2 u_screenSize;

vec3 toNDC(vec4 worldPos) {
    vec4 clip = u_projectionMatrix * u_viewMatrix * worldPos;
    return clip.xyz / clip.w;
}

float estimateScreenSpaceTess(vec4 p0, vec4 p1, vec4 p2, vec4 p3) {
    vec3 ndc0 = toNDC(p0);
    vec3 ndc1 = toNDC(p1);
    vec3 ndc2 = toNDC(p2);
    vec3 ndc3 = toNDC(p3);

    vec3 mini = min(min(min(ndc0, ndc1), ndc2), ndc3); 
    vec3 maxi = max(max(max(ndc0, ndc1), ndc2), ndc3); 
    
    float sizeX = abs((maxi.x - mini.x) * u_screenSize.x);
    float sizeY = abs((maxi.y - mini.y) * u_screenSize.y);

    return (sizeX * sizeY) * 4;
}


void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    float tessLevel = clamp(estimateScreenSpaceTess(p0, p1, p2, p3), 8.0, 64.0);
    
    gl_TessLevelOuter[0] = 1.0; 
    gl_TessLevelOuter[1] = tessLevel;      
}
