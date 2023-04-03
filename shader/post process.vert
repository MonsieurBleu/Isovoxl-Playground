#version 430

layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;

in vec3 gpu_Vertex;
in vec2 gpu_TexCoord;
in vec4 gpu_Color;
uniform mat4 gpu_ModelViewProjectionMatrix;

out vec2 texCoord;

void main (void)
{
    texCoord = gpu_TexCoord;
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1);
}