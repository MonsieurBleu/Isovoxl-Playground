#version 430

#define PI 3.1415926535897932384626433832795

layout (location = 1) uniform float Time;
layout (location = 5) uniform ivec4 win_const;
// in vec4 color;

in vec4 texColor;
in vec2 texCoord;
uniform sampler2D world;

out vec4 fragColor;

bool highlight1;
bool highlight2;
bool highlighterr;

uvec4 flags;

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

vec2 uv;
float ratio = win_const.y/1920.0;
float corner_size = 25.0 * ratio;
float border_size = 10.0 * ratio;
// float border_size2 = border_size/4.0;
float border_size2 = 0.0;
// const float corner_size = 300.0;
bool is_in_corner()
{
    uv = uv*win_const.ba;

    bool xcorner = uv.x < corner_size || uv.x > (win_const.b-corner_size);
    bool ycorner = uv.y < corner_size || uv.y > (win_const.a-corner_size);

    return xcorner && ycorner;
}

vec4 circle(vec2 uv, vec2 pos, float rad, vec3 color) {
	float d = length(pos - uv) - rad;
	float t = clamp(d, 0.0, 1.0);
	return vec4(color, 1.0 - t);
}



void main (void)
{
    flags.a = int(texColor.a*256.0);

    highlight1 = ((flags.a&128) != 0);
    highlight2 = ((flags.a&64) != 0);
    highlighterr = ((flags.a&32) != 0);

    bool border = false;
    float border_val = 0.0;

    uv = texCoord;

    float bg_alpha = 0.10;


    // blanc - noir 
    // vec4 pixel_bg = vec4(245.0, 245.0, 245.0, 256.0)/256.0;
    // vec4 pixel_border = vec4(65.0, 65.0, 65.0, 0.0)/256.0;

    // gris- noir
    // vec4 pixel_bg = vec4(75.0, 75.0, 75.0, 256.0)/256.0;
    // vec4 pixel_border = vec4(45.0, 45.0, 45.0, 0.0)/256.0;

    // pêche - blanc
    // vec4 pixel_bg = vec4(255.0, 161.0, 123.0, 256.0)/256.0;
    // vec4 pixel_border = vec4(245.0, 245.0, 245.0, 0.0)/256.0;

    // bleu-vert pastelle
    // vec4 pixel_bg = vec4(215.0, 236.0, 230.0, 256.0)/256.0;
    // vec4 pixel_border = vec4(69.0, 69.0, 69.0, 0.0)/256.0;

    // blanc-noir 2
    vec4 pixel_bg = vec4(215.0, 215.0, 215.0, 256.0)/256.0;
    vec4 pixel_border = vec4(69.0, 69.0, 69.0, 256.0)/256.0;

    if(highlight2)
    {
        pixel_bg = vec4(250.0, 250.0, 250.0, 256.0)/256.0;
        pixel_border = vec4(100.0, 100.0, 100.0, 256.0)/256.0;
    }

    if(highlight1)
    {
        // border_size *= 0.75;
        pixel_bg = vec4(33.0, 150.0, 243.0, 256.0)/256.0;
        pixel_border = vec4(34.0, 84.0, 142.0, 256.0)/256.0;

        if(highlight2)
        {
            pixel_bg *= 1.25;
            pixel_border *= 1.75;
        }
    }   

    if(highlighterr)
    {
        pixel_bg = vec4(250.0, 128.0, 128.0, 256.0)/256.0;
        pixel_border = vec4(100.0, 50.0, 50.0, 256.0)/256.0;

        if(highlight2)
        {
            pixel_bg *= 1.25;
            pixel_border *= 1.75;
        }
    }

    if((flags.a&16) != 0)
    {
        // pixel_bg = vec4(128.0, 250.0, 128.0, 256.0)/256.0;
        pixel_bg = vec4(55.0, 226.0, 156.0, 256.0)/256.0;
        pixel_border = vec4(50.0, 100.0, 50.0, 256.0)/256.0;

        if(highlight2)
        {
            pixel_bg *= 1.25;
            pixel_border *= 1.75;
        }
    }

    if((flags.a&8) != 0)
    {
        border_size = -1.0;
    }

    if(is_in_corner())
    {
        pixel_bg.a = 0.0;
        vec2 center;

        if(uv.x <= corner_size)
            center.x = corner_size;
        else
            center.x = win_const.b-corner_size;

        if(uv.y <= corner_size)
            center.y = corner_size;
        else
            center.y = win_const.a-corner_size;
        
        float radius  = corner_size;
        float radius2 = corner_size-border_size-0.5;

        // Circle
        pixel_bg     = circle(uv, center, radius2, pixel_bg.rgb);
        pixel_border = circle(uv, center, radius, pixel_border.rgb);
    }
    else
    {
        if(
            min(uv.x, uv.y) < border_size ||
            min(abs(win_const.b-uv.x), abs(win_const.a-uv.y)) < border_size
        )
        {
            pixel_bg.a = 0.0;
        }
    }

    fragColor.rgb = (pixel_bg.rgb*pixel_bg.a) + (pixel_border.rgb*(1.0-pixel_bg.a));
    pixel_bg.a *= bg_alpha;
    fragColor.a = pixel_border.a-pixel_bg.a;
}