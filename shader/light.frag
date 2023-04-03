#version 430

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8

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

uint render_flagsl;
uint render_flagsr;
uint render_flagst;
uint shadows_flags;
vec2 texCoord;

uint id;
int block_height;
float PixelTint;
uint face;

in vec2 MozCoord;
in flat uint block_info;
in flat uint render_flags;

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D atlas;
layout (binding = 1) uniform sampler2D block_atlas;
// layout (binding = 2) uniform sampler2D ao;
layout (binding = 3) uniform sampler2D DFIB;
// layout (binding = 4) uniform sampler2D water;
// layout (binding = 5) uniform sampler2D border;
// layout (binding = 6) uniform sampler2D normal;
// layout (binding = 8) uniform sampler2D world;
// layout (binding = 9) uniform sampler2D light;

int extract_depth(vec4 DFIB)
{
    return int(DFIB.r*256.0) + (int(DFIB.g*256.0)/8)*256;
}

void handle_depth()
{
    gl_FragDepth = 1-(float(block_height)+8.0)/8192;
}

#define PI 3.1415926535897932384626433832795

vec2 coord_roration(vec2 uv, vec2 rpoint, float angle)
{
    angle = angle * PI/180;
    return vec2(
           (uv.x-rpoint.x)*cos(angle)-(uv.y-rpoint.y)*sin(angle) + rpoint.x, 
           (uv.x-rpoint.x)*sin(angle)+(uv.y-rpoint.y)*cos(angle) + rpoint.y);
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

void main (void)
{
    vec4 pixel;

    id = block_info%256;
    block_height = int(block_info>>8);

    vec2 ColorCoord = vec2(float(id%16)/16.0, float(id/16)/16.0);
    pixel = texture(block_atlas, ColorCoord);

    texCoord.x = float(int(MozCoord.x*atlas_size)%block_size)/block_size;
    texCoord.y = float(int(MozCoord.y*atlas_size)%block_size)/block_size;
    pixel.a = texture(atlas, texCoord).a;

    // test debug
    // pixel = vec4(vec3(1.0), pixel.a);
    // pixel.rgb = rgb2hsv(pixel.rgb);
    // pixel.r = (cos(Time/5)+1.0)*0.5;
    // pixel.g = 1.0;
    // pixel.b = 1.0;
    // pixel.rgb = hsv2rgb(pixel.rgb);
    ///

    vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, gl_FragCoord.y/win_const.y);
    vec4 DFIBp;
    DFIBp = texture(DFIB, screen_pos);
    int depth = extract_depth(DFIBp);

    // tentative 1 (en conaissant la hauteur réelle du block)
        float diff2 = float(abs(depth-block_height))*0.05;
        float signd = diff2 < 0 ? -1.0 : 1.0;
        
        vec2 uv2 = texCoord;

        uv2 = uv2*2;
        uv2 -= 0.5;
        // uv2.x = uv2.x/2 + 0.25;
        uv2.x = uv2.x/1.5 + 1/6.0;

        float val = pow(diff2, 0.5);

        val = diff2*0.25;

        // val = pow(val, 0.5);

        fragColor.a = texture(atlas, uv2).a*0.5 - val;
        // fragColor.a *= 0.5;
        // fragColor.a = 1-fragColor.a;
        fragColor.rgb = pixel.rgb;

    // tentative 2 : trouver la hauteur réelle du block

        // depth += int(32*(texCoord.x-0.5)); 
        // depth += int(32*(texCoord.y-0.5)); 

        // float diff2 = float(abs(depth-block_height+1))*0.05;
        // float signd = diff2 < 0 ? -1.0 : 1.0;
        
        // vec2 uv2 = texCoord;

        // uv2 = uv2*2;
        // uv2 -= 0.5;
        // uv2.x = uv2.x/2 + 0.25;

        // fragColor.a = texture(atlas, uv2).a - pow(diff2, 0.5);
        // fragColor.rgb = vec3(1.0);

}