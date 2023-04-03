#version 430

layout (points) in;
layout (triangle_strip, max_vertices = 128) out;
// layout (triangle_strip, max_vertices = 128) out;

uniform mat4 gpu_ModelViewProjectionMatrix;
uniform sampler2D tex;

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;
layout (location = 9) uniform int max_height_render;

in flat int Vid[];
in flat uvec4 Vcolor[];
in ivec2 block_position[];
in flat uint Vblock_height[];
in flat uint projection_grid_face[];
in flat int Vline_size[];
in flat uint Vrenderf[];

int render_flagsl;
int render_flagsr;
int render_flagst;
int shadows_flags;
int id;

int line_size;

out vec2 MozCoord;
out flat uint block_info;
out flat uint render_flags;

void manage_outpouts(int i)
{
    id = Vid[0];
    block_info = id + (Vblock_height[0]<<8);

    // render_flags = Vcolor[0].r + (Vcolor[0].g<<8) + (Vcolor[0].b<<16) + (Vcolor[0].a<<24);
    render_flags = Vrenderf[0];
}

void main()
{
    manage_outpouts(0);
    line_size = Vline_size[0];

    // if(projection_grid_face == 0)
    // line_size = 0;

    int x = int(block_position[0].x);
    int y = int(block_position[0].y);
    int z = int(Vblock_height[0]);

    // x = 0;
    // y = 0;

    float sprite_size_debug = sprite_size;

    sprite_size_debug *= 32.0;

    for(int i = 0; i <= line_size; i++)
    {
        vec4 VPos = vec4(0.0, 0.0, 0.0, 1.0);

        for(int j = 0; j < 2; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                VPos.x = win_const.b + (sprite_size/2)*(x-y) - sprite_size/2;
                VPos.y = win_const.a + (sprite_size/4)*(x+y-2*z) - sprite_size/2;

                MozCoord = vec2(0.0);

                // uint finalx = x;
                // uint finaly = y;
                if(projection_grid_face[0] == 2 || projection_grid_face[0] == 0)
                {
                    // axe y
                    VPos.x += (sprite_size/2.0)*(-i);
                    VPos.y += (sprite_size/4.0)*(i);

                    // finaly = y+i;
                }
                if(projection_grid_face[0] == 1)
                {
                    // axe x
                    VPos.x += (sprite_size/2.0)*(i);
                    VPos.y += (sprite_size/4.0)*(i);

                    // finalx = x+i;
                }

                if(j == 0)
                {
                    if(k > 0)
                    {
                        VPos.y += sprite_size_debug;
                        MozCoord.y = 1.0;
                    }

                    if(k == 2)
                    {
                        VPos.x += sprite_size_debug;
                        MozCoord.x = 1.0;
                    }
                }
                else
                {
                    if(k > 0)
                    {
                        VPos.x += sprite_size_debug;
                        MozCoord.x = 1.0;
                    }

                    if(k == 2)
                    {
                        VPos.y += sprite_size_debug;
                        MozCoord.y = 1.0;
                    }
                }

                float decval = -sprite_size/2.0 + sprite_size_debug/2.0;
                VPos.x -= decval;
                VPos.y -= decval - sprite_size/4.0;

                gl_Position = gpu_ModelViewProjectionMatrix * VPos;
                
                uint depth = z+1;
                block_info = id + (depth <<8);

                EmitVertex();
            }

            EndPrimitive();
        }
    }
}