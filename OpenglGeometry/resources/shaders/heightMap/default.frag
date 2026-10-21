#version 460 core

in vec3 v_localPos;
in vec3 v_localNormal;
in vec3 v_worldPos;

uniform mat4 u_modelMatrix = mat4(1.0);
uniform vec3 u_lightDir = normalize(vec3(0.4, 1.0, 0.3)); // direction towards light
uniform vec4 u_color = vec4(0.8);

out vec4 FragColor;

void main()
{
    // Flat face normal in local space
    vec3 faceN = normalize(cross(dFdx(v_localPos), dFdy(v_localPos)));

    // Top triangles always tilt upward; walls (y == 0) and bottom (y < 0) don't
    vec3 localN = faceN.y > 0.01 ? normalize(v_localNormal) : faceN;

    // Better: pass this in as a uniform computed on the CPU
    mat3 normalMatrix = transpose(inverse(mat3(u_modelMatrix)));
    vec3 N = normalize(normalMatrix * localN);

    float diff = max(dot(N, normalize(u_lightDir)), 0.0);
    FragColor = vec4(u_color.xyz * (0.15 + 0.85 * diff), 1.0);
}