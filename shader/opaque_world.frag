#version 430

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8
#define SFEATURE_BLOOM               16
#define SFEATURE_GRID                32

#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;
layout (location = 9) uniform int max_height_render;

in vec2 texCoord;
layout (binding = 1) uniform sampler2D atlas;
layout (binding = 3) uniform sampler2D DFIB;
layout (binding = 7) uniform sampler2D light;

out vec4 fragColor;

uint id;
float PixelTint;
uint face;
int depth;
bool border;

void extract_id(vec4 DFIB)
{
    id = uint(DFIB.a*256.0);
}

void extract_border(vec4 DFIB)
{
    uint b = int(DFIB.g*256.0) & 4;

    if(b == 0)
        border = false;
    else
        border = true;
}

float extract_Brightness(vec4 DFIB)
{
    return DFIB.b;
}

uint extract_face(vec4 DFIB)
{
    uint eface = (int(DFIB.g*256.0)<<30)>>30; // modulo didn't work here for some reasons

    if(eface != 0 && eface != 1)
        eface = 2;
    
    return eface;
}

int extract_depth(vec4 DFIB)
{
    return int(DFIB.r*256.0) + (int(DFIB.g*256.0)/8)*256;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

bool test_border(vec2 uv)
{
    vec4 DFIB2 = texture(DFIB, uv);
    uint face2 = extract_face(DFIB2);

    if(face != face2)
    {
        return true;
    }

    int depth2 = extract_depth(DFIB2);

    if(abs(depth-depth2) > 3)
        return true;

    return false;
}

float handle_border2()
{
    float border_color = 1.5;
    float size = 0.000005* sprite_size;
    vec2 uv;

    uv = texCoord;
    uv.x += size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.y += size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.y -= size;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x += size;
    uv.y += size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    uv.y += size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x += size;
    uv.y -= size*2;
    if(test_border(uv)) return border_color;

    uv = texCoord;
    uv.x -= size;
    uv.y -= size*2;
    if(test_border(uv)) return border_color;

    return 1.0;
}

float handle_border3(float brightness)
{
    float border_color = 1.5;

    border_color = 1.0 + (global_illumination.r + global_illumination.g + global_illumination.b + brightness)/8.0;

    if(border)
        return border_color;

    return 1.0;
}

void main (void)
{
    /// debug
    // vec4 pl_debug = texture(light, texCoord);
    // fragColor = pl_debug;
    // return;

    ///// INIT DFIB /////
    vec4 DFIB = texture(DFIB, texCoord);
    extract_id(DFIB);

    id --;

    if(id > 256) discard;

    ///// PIXEL TINT /////
    vec4 pixel_light = texture(light, texCoord);
    pixel_light *= 2;
    // pixel_light *= 0.5;
    // pixel_light = sqrt(pixel_light);

    // float maxc = max(pixel_light.r, pixel_light.g);
    // maxc = max(maxc, pixel_light.b);

    // float lim = 0.95;
    // if(pixel_light.b > lim) pixel_light.b = lim;
    // if(pixel_light.r > lim) pixel_light.g = lim;
    // if(pixel_light.g > lim) pixel_light.b = lim;

    face = extract_face(DFIB);

    // float GI = 1.0;
    float b = extract_Brightness(DFIB);

    float GI = light_direction[face];

    if(id >= 208)
        GI = GI + (1.0-GI)*0.85;
    
    // vec3 test = global_illumination.rgb + pixel_light.rgb*0.5;
    vec3 test = global_illumination.rgb + pixel_light.rgb*0.5;

    if(test.r >= 1) test.r = 1;
    if(test.g >= 1) test.g = 1;
    if(test.b >= 1) test.b = 1;
    if(id >= 208) test = vec3(1.0);

    vec4 PixelTint = vec4(test, 1);
    
    // if(b != 1)
    // {
    //     float GI_avg = (global_illumination.r + global_illumination.g + global_illumination.b)/3.0;

    //     GI_avg += 0.15;

    //     b *= 1.0/(GI_avg);
        
    //     float maxb = 1.0;

    //     if(b > maxb) b = maxb;

    //     // if(b <= 0.25) b = 0.25;
    // }

    // if(b <= 1.0)
    //     b += (pixel_light.r + pixel_light.g + pixel_light.b)/3.0 * pixel_light.a * 5;
    // if(b > 1.0) b == 1.0;

    // float val = (pixel_light.r + pixel_light.g + pixel_light.b)/3.0;
    // if(val != 0.0)
    //     b *= 1.0+val;

    if(id >= 208)
        PixelTint *= b*1.75;
    else
        PixelTint *= b;

    PixelTint *= GI;

    ///// COLOR /////
    vec2 ColorCoord = vec2(float(id%16)/16.0, float(id/16)/16.0);
    vec4 block_color = texture(atlas, ColorCoord);

    ///// BORDER /////

    if(sprite_size > 5)
    {
        depth = extract_depth(DFIB);
        extract_border(DFIB);

        if((features & SFEATURE_GRID) == 0)
            PixelTint *= handle_border3(b);
        else if(border)
        {
            block_color = 1.0 - block_color;
            
            block_color.rgb = rgb2hsv(block_color.rgb);
            block_color.g = 1.0;
            block_color.b = 1.0;
            block_color.rgb = hsv2rgb(block_color.rgb);

            PixelTint = vec4(1.0);
        }
    }

    ///// FINAL /////

    // pixel_light = pixel_light - PixelTint;

    // PixelTint *= 1.0 + pixel_light;

    fragColor = block_color * PixelTint;

    fragColor.a = 1.0;
    gl_FragDepth = 0;

    //debug
    // fragColor.rgb = vec3(depth/512.0);
}