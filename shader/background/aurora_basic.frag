#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

layout (location = 16) uniform sampler2D iChannel0;

in vec2 texCoord;

out vec4 fragColor;

float iTime = Time/750.0;

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

void draw_auroras(inout vec4 color, vec2 uv)
{
    // const vec4 aurora_color_a = vec4(0.0, 1.2, 0.5, 1.0);
    // const vec4 aurora_color_b = vec4(0.0, 0.4, 0.6, 1.0);

    const vec4 aurora_color_a = vec4(0.0, 1.2, 0.5, 1.0);
    const vec4 aurora_color_b = vec4(0.0, 0.4, 0.6, 1.0);
    
    float t = nsin(-iTime + uv.x * 100.0) * 0.075 + nsin(iTime + uv.x * distance(uv.x, 0.5) * 100.0) * 0.1 - 0.5;
    t = 1.0 - smoothstep(uv.y - 4.0, uv.y * 2.0, t);
    
    vec4 final_color = mix(aurora_color_a, aurora_color_b, clamp(1.0 - uv.y * t, 0.0, 1.0));
    final_color += final_color * final_color;
    color += final_color * t * (t + 0.5) * 0.75;
}

void main()
{
    // vec2 ps = vec2(1.0, 1.0) / win_const.xy;
    vec2 uv = vec2(texCoord.x, 1-texCoord.y);
    fragColor = vec4(0.0025, 0.075, 0.10, 1.0)*0.5;
    
    fragColor.a = 1.0;

    draw_auroras(fragColor, uv);
}
