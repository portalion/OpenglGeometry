#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 a_patchInfo;

out vec4 vPatchInfo;

void main()
{
    vPatchInfo = a_patchInfo;
    gl_Position = position;
};
