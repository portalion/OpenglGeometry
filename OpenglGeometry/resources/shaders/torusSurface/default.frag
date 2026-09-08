#version 460 core

out vec4 fragColor;

in vec2 v_uv;

uniform vec4 u_color = vec4(0.0, 0.0, 1.0, 1.0);
uniform sampler2D u_trimTex;
uniform int u_shouldTrim = 0;
uniform int u_keepFilled = 1;

void main()
{
    if (u_shouldTrim != 0)
    {
        bool filled = texture(u_trimTex, v_uv).r > 0.5;
        if ((u_keepFilled != 0) != filled)
        {
            discard;
        }
    }
    fragColor = u_color;
}
