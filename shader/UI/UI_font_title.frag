#version 430

#define PI 3.1415926535897932384626433832795

layout (location = 1) uniform float Time;
layout (location = 5) uniform ivec4 win_const;
// in vec4 color;

in vec4 texColor;
in vec2 texCoord;
uniform sampler2D world;

out vec4 fragColor;

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
    vec4 pixel = texture(world, texCoord);

    // pixel.rgb = rgb2hsv(pixel.rgb);
    // pixel.r = (sin((Time/2.0) + (gl_FragCoord.x/win_const.x))+0.5)*0.5;
    // pixel.g = 0.85;
    // pixel.b = 0.85;
    // pixel.rgb = hsv2rgb(pixel.rgb);

    float cx = (gl_FragCoord.x-win_const.w)/win_const.x;

    float val = (sin((Time) + cx) + 1.0)*0.5;
    // float val = 1.0;
    float val2 = sin(Time)*0.25 + 0.25;
    float debug = sin(Time+1000)*0.25 + 0.25;

    // if(val < 0.005)
    // if(val > 0.475 && val < 0.525)
    if(val > 0.995)
    {
        pixel.rgb = vec3(0.75);
    }
    
    // if(val2 > 0.45 && val2 < 0.55)
    float beg = 0.20;
    float end = 0.30;
    float diff = end-beg;
    if(val2 > beg && val2 < end && val2 > debug)
    {
        // 0.6*x = PI
        // 0.6 = PI/x

        // float val3 = (sin((val2-0.45)*(PI/0.10)) + 1.0)*0.5;
        float val3 = (sin((val2-beg)*(PI/diff)) + 1.0)*0.5;
        pixel.rgb = vec3(val3*0.75);

        // pixel.g = val;
        // pixel.rgb += vec3(val2/2.0);
    }


    fragColor = pixel;

    gl_FragDepth = 0;
}