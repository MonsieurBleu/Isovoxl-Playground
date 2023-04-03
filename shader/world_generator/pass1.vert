#version 430

layout (location = 5) uniform ivec4 win_const;

in vec3 gpu_Vertex;
in vec2 gpu_TexCoord;
in vec4 gpu_Color;
uniform mat4 gpu_ModelViewProjectionMatrix;

out vec4 texColor;
out vec2 texCoord;

void main (void)
{
    texColor = gpu_Color;
    texCoord = gpu_TexCoord;
    gl_Position = gpu_ModelViewProjectionMatrix * gpu_Vertex;
}