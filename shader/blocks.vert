#version 430

in vec3 gpu_Vertex;
in vec2 gpu_TexCoord;
in vec4 gpu_Color;
uniform mat4 gpu_ModelViewProjectionMatrix;

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;
layout (location = 9) uniform int max_height_render;

out flat int Vid;
out flat uvec4 Vcolor;
out ivec2 block_position;
out flat uint Vblock_height;
out flat uint projection_grid_face;
out flat int Vline_size;
out flat uint Vrenderf;

uint select_bits(uint val, uint beg, uint size)
{
    return (val>>beg)%(1<<size);
}

void main(void)
{
    uint u1 = floatBitsToUint(gpu_Vertex.x);
    uint u2 = floatBitsToUint(gpu_Vertex.y);
    uint u3 = floatBitsToUint(gpu_Vertex.z);

    Vcolor.a = select_bits(u1, 0, 8);
    Vcolor.b = select_bits(u1, 8, 8);
    Vcolor.g = select_bits(u1, 16, 8);
    Vcolor.r = select_bits(u1, 24, 8);

    Vrenderf = u1;

    block_position.x = int(select_bits(u2, 0, 16));
    block_position.y = int(select_bits(u2, 16, 16));

    Vblock_height = select_bits(u3, 0, 16);
    Vid = int(select_bits(u3, 16, 8))-1;

    projection_grid_face = select_bits(u3, 24, 2);
    Vline_size = int(select_bits(u3, 26, 6));
}