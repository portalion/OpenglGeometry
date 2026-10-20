#version 460 core

layout(location = 0) out vec4 fragColor;

in vec3 v_worldPos;
in vec3 v_normal;

uniform vec4 u_color = vec4(0.85, 0.75, 0.55, 1.0);
uniform vec4 g_cameraPosition;

const vec3 c_lightDirection = normalize(vec3(0.4, 1.0, 0.3));
const float c_ambient = 0.2;

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 toCamera = normalize(g_cameraPosition.xyz - v_worldPos);

    float key = max(dot(normal, c_lightDirection), 0.0);
    float head = max(dot(normal, toCamera), 0.0);
    float lighting = c_ambient + 0.5 * key + 0.4 * head;

    fragColor = vec4(u_color.rgb * lighting, u_color.a);
}
