#version 430

#define PI 3.1415926535897932384626433832795

layout (location = 1) uniform float Time;
layout (location = 5) uniform ivec4 win_const;
// in vec4 color;

in vec4 texColor;
in vec2 texCoord;
uniform sampler2D world;

out vec4 fragColor;

void main (void)
{
    vec4 pixel = texture(world, texCoord);
    
    float cx = (gl_FragCoord.x-win_const.z)/win_const.x;
    float cy = (gl_FragCoord.y-win_const.a)/win_const.y;

    if(pixel.a == 0) discard;

    // fragColor.rgb = pixel.rgb + 0.40;
    // fragColor.a = 1.0;

    fragColor = vec4(0.75, 0.40, 0.40, 1.0);
}