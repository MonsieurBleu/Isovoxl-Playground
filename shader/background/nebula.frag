#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

layout (location = 16) uniform sampler2D iChannel0;

in vec2 texCoord;

out vec4 fragColor;

float iTime = Time/1000.0;

float iSpeed = 50.0;
float iDensity = 2.0;
float iStarSize = 4.0;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy,vec2(12.9898,78.233)))*43758.5453123);
}

void main()
{
    vec2 filteredRes = vec2(gl_FragCoord.x - mod(gl_FragCoord.x, iStarSize), gl_FragCoord.y - mod(gl_FragCoord.y, iStarSize));
    vec2 st = filteredRes / win_const.xy;
    st *= 10.0;

    vec2 ipos = floor(st);
    vec2 fpos = fract(st);

    float isStar = (pow(rand(fpos), 100.0 / iDensity)) * sin((iTime/(100.0/iSpeed)) * (rand(filteredRes) * (3.14159)));
    float nebula = (rand(fpos) ) * 0.2;
    fragColor = vec4(max(isStar, (sin(nebula) + 1.0)/8.0), isStar, max(isStar, (cos(nebula) + 1.0)/8.0), 1.0);
}