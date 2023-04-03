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
    Vline_size = 0;
}

// #version 430

// in vec3 gpu_Vertex;
// in vec2 gpu_TexCoord;
// in vec4 gpu_Color;
// uniform mat4 gpu_ModelViewProjectionMatrix;

// layout (location = 1) uniform float Time;
// layout (location = 2) uniform int features;
// layout (location = 3) uniform vec3 light_direction;
// layout (location = 4) uniform vec4 global_illumination;
// layout (location = 5) uniform ivec4 win_const;
// layout (location = 6) uniform float sprite_size;
// layout (location = 7) uniform int block_size;
// layout (location = 8) uniform int atlas_size;
// layout (location = 9) uniform int max_height_render;

// uniform sampler2D ao;

// out flat int Vid;
// out vec4 Vcolor;
// out vec2 VMozCoord;
// out ivec2 corner;
// out ivec2 block_position;
// out vec3 final_Vertex;
// out flat uint Vblock_height;

// out flat int Vline_size;

// // uniform sampler2D world;

// int block_per_atlas_line;

// void main(void)
// {
//     Vline_size = 0;
    
//     Vcolor = gpu_Color;

//     vec2 final_TextCoord = gpu_TexCoord;

//     corner = ivec2(0);
//     if(gpu_TexCoord.x >= 1)
//         corner.x = 1;
//     if(gpu_TexCoord.y >= 1)
//         corner.y = 1;

//     Vid = int((gpu_TexCoord.x-corner.x)*atlas_size)%256;

//     if(corner.x == 1)
//     {
//         Vline_size = int((gpu_TexCoord.x-1)*atlas_size-Vid)/256;
//     }

//     block_per_atlas_line = atlas_size/block_size;
//     float block_size_incoord = 1.0/(1.0*block_per_atlas_line);
//     final_TextCoord.x = (Vid%block_per_atlas_line + corner.x)*block_size_incoord;
//     final_TextCoord.y = ((Vid>>4) + corner.y)*block_size_incoord;

//     VMozCoord = vec2(final_TextCoord.x, final_TextCoord.y);
    
//     Vblock_height = uint((gpu_TexCoord.y-corner.y)*atlas_size);

//     final_Vertex = gpu_Vertex;

//     int x = int(gpu_Vertex.x);
//     int y = int(gpu_Vertex.y);

//     // float light_size = (sin(Time)+1.0)*16.0 + 16.0;
//     float light_size = 32.0;

//     final_Vertex.x = win_const.b + (sprite_size/2)*(x-y) + sprite_size*light_size*corner.x - light_size*sprite_size/2;
//     final_Vertex.y = win_const.a + (sprite_size/4)*(x+y) + sprite_size*light_size*corner.y - light_size*sprite_size/2;

//     block_position = ivec2(x, y);

//     gl_Position = gpu_ModelViewProjectionMatrix * vec4(final_Vertex, 1);
// }