#shader vertex
#version 410 core

layout(location = 0) in vec4 position;
layout(location = 0) out vec4 color;

void main()
{
    gl_Position = position;
};

#shader fragment
#version 410 core

layout(location = 0) in vec4 in_Color;
layout(location = 0) out vec4 color;

void main()
{
   color = in_Color;
};