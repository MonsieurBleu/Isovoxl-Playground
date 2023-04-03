#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

layout (location = 16) uniform sampler2D iChannel0;
layout (location = 17) uniform uint data;

in vec2 texCoord;

out vec4 fragColor;

float iTime = Time/1000.0;

// https://www.shadertoy.com/view/wtK3zG

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


#define nsin(x) (sin(x) * 0.5 + 0.5)

vec4 aurora_color_a = vec4(0.0, 0.0, 0.0, 1.0);
vec4 aurora_color_b = vec4(0.0, 0.0, 0.0, 1.0);

vec4 draw_auroras(vec4 color, vec2 uv)
{
    // const vec4 aurora_color_a = vec4(0.0, 1.2, 0.5, 1.0);
    // const vec4 aurora_color_b = vec4(0.0, 0.4, 0.6, 1.0);
    
    float t = nsin(-iTime + uv.x * 100.0) * 0.075 + nsin(iTime + uv.x * distance(uv.x, 0.5) * 100.0) * 0.1 - 0.5;
    t = 1.0 - smoothstep(uv.y - 4.0, uv.y * 2.0, t);
    
    vec4 final_color = mix(aurora_color_a, aurora_color_b, clamp(1.0 - uv.y * t, 0.0, 1.0));
    final_color += final_color * final_color;
    color += final_color * t * (t + 0.5) * 0.75;

    return color;
}

float iSpeed = 25.0;
float iDensity = 2.0;
float iStarSize = 4.0;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy,vec2(12.9898,78.233)))*43758.5453123);
}

void main()
{
    vec2 fg = gl_FragCoord.xy;

    // fg.x -= iTime*10;
    // fg.y -= iTime*10;

    fg += (data>>30)*win_const.x;

    vec2 filteredRes = vec2(fg.x - mod(fg.x, iStarSize), fg.y - mod(fg.y, iStarSize));
    vec2 st = filteredRes / win_const.xy;

    st *= 10.0;

    vec2 ipos = floor(st);
    vec2 fpos = fract(st);

    float isStar = (pow(rand(fpos), 100.0 / iDensity)) * sin((iTime/(100.0/iSpeed)) * (rand(filteredRes) * (3.14159)));
    float nebula = (rand(fpos) ) * 0.2;
    vec4 pixel_stars = vec4(max(isStar, (sin(nebula) + 1.0)/8.0), isStar, max(isStar, (cos(nebula) + 1.0)/8.0), 1.0);

    if((data&128) == 0)
    {
        switch(data%4)
        {
            case 0 : // AZUR
                aurora_color_a = vec4(0.0, 1.2, 0.5, 1.0);
                aurora_color_b = vec4(0.0, 0.4, 0.6, 1.0);
                break;

            case 1 : // ORCHID
                aurora_color_a = vec4(1.5, 0.35, 0.90, 1.0);
                aurora_color_b = vec4(0.5, 0.15, 0.15, 1.0);
                break;

            case 2 : // SCARLET
                aurora_color_a = vec4(1.75, 1.2, 0.2, 1.0);
                aurora_color_b = vec4(0.5, 0.15, 0.05, 1.0);
                break;
            
            case 3 : // JADE
                aurora_color_a = vec4(0.25, 1.25, 0.25, 0.0);
                aurora_color_b = vec4(0.1, 0.75, 0.1, 0.0);
                break;
        }

        fg = gl_FragCoord.xy/win_const.xy;
        vec2 uv = vec2(fg.x, 1-fg.y);;
        // vec2 uv = vec2(texCoord.x, 1-texCoord.y);
        vec4 pixel_aurora = draw_auroras(vec4(0.0), uv);

        vec3 psh = rgb2hsv(pixel_stars.rgb);

        if(pixel_stars.g < 0)
            pixel_stars.g = 0;

        fragColor.rgb = pixel_aurora.rgb + pixel_stars.rgb;
        fragColor.a = 1.0;
    }
    else
    {
        fragColor = pixel_stars;
    }
}