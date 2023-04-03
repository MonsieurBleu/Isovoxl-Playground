#version 430

#define PI 3.1415926535897932384626433832795

layout (location = 1) uniform float Time;
layout (location = 5) uniform ivec4 win_const;
// in vec4 color;

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
    
    // float cx = (gl_FragCoord.x-win_const.w)/win_const.x;
    float cx = (gl_FragCoord.x-win_const.z)/win_const.x;
    float cy = (gl_FragCoord.y-win_const.a)/win_const.y;

    if(pixel.a != 0)
    {
        fragColor.rgb = vec3(0.85);


        // vec3 color1 = vec3(1, 0, 1);
        // vec3 color2 = vec3(0, 0, 1);

        // cy = sin(cy + Time);

        // fragColor.rgb = cy*color1 + (1-cy)*color2;

        fragColor.a = pixel.a;
    }

    vec2 testcoord = texCoord.xy;
    testcoord.y += 0.010;
    testcoord.x += 0.0010;
    vec4 pixel2 = texture(world, testcoord);
    if(pixel2.a == 0 && pixel.a != 0)
    {
        fragColor.rgb = vec3(0.25);
        fragColor.a = 1.0;
    }


    // fragColor = pixel;

    // fragColor = vec4(cx, cy, 0.0, 1.0);
    gl_FragDepth = 0;
}