#version 430

#define SFEATURE_BLOOM 16
#define SFEATURE_GRID  32

layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;

in vec2 texCoord;
uniform sampler2D iChannel0;

out vec4 fragColor;

// bloom actuel : https://www.shadertoy.com/view/lsXGWn
// pk pas tester : https://www.shadertoy.com/view/MtfSDH ou https://www.shadertoy.com/view/flK3zy ou https://www.shadertoy.com/view/lsXGWn
const float blurSize = 1.0/512.0;
const float intensity = 0.10;
void bloom()
{
   vec4 sum = vec4(0);
   int j;
   int i;

   //thank you! http://www.gamerendering.com/2008/10/11/gaussian-blur-filter-shader/ for the 
   //blur tutorial
   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(iChannel0, vec2(texCoord.x - 4.0*blurSize, texCoord.y)) * 0.05;
   sum += texture(iChannel0, vec2(texCoord.x - 3.0*blurSize, texCoord.y)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x - 2.0*blurSize, texCoord.y)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x - blurSize, texCoord.y)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y)) * 0.16;
   sum += texture(iChannel0, vec2(texCoord.x + blurSize, texCoord.y)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x + 2.0*blurSize, texCoord.y)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x + 3.0*blurSize, texCoord.y)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x + 4.0*blurSize, texCoord.y)) * 0.05;
	
	// blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 4.0*blurSize)) * 0.05;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 3.0*blurSize)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - 2.0*blurSize)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y - blurSize)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y)) * 0.16;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + blurSize)) * 0.15;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 2.0*blurSize)) * 0.12;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 3.0*blurSize)) * 0.09;
   sum += texture(iChannel0, vec2(texCoord.x, texCoord.y + 4.0*blurSize)) * 0.05;

   //increase blur with intensity!
   fragColor = sum*intensity + texture(iChannel0, texCoord); 
   
   /*
   if(sin(iTime) > 0.0)
       fragColor = sum * sin(iTime)+ texture(iChannel0, texCoord);
   else
	   fragColor = sum * -sin(iTime)+ texture(iChannel0, texCoord);
    */
}

// void is_near(vec2 value, vec2 target, float nprecision)
// {
//     if(value.x > target.x-nprecision && value.x < target.x-nprecision)
// }

void drawline()
{
    // fragColor.rg = vec2(1.0);
    // fragColor.a = 0.5;
}

#define PI 3.1415926535897932384626433832795
vec2 coord_roration(vec2 uv, float angle)
{
    angle = angle * PI/180;
    return vec2(uv.x*cos(angle)-uv.y*sin(angle), uv.x*sin(angle)+uv.y*cos(angle));
}

const float line_width = 1.0;
// const float nb_lines = sprite_size;
void isometric_grid()
{
    vec2 uv = gl_FragCoord.xy;

    uv.x /= 2.0;
    uv = coord_roration(uv, 45.0);

    // uv /= nb_lines;
    // vec2 uvf = vec2(floor(uv.x), floor(uv.y));
    // uv *= nb_lines;
    // uvf *= nb_lines;
    // if(abs(uv.x-uvf.x) < line_width || abs(uv.y-uvf.y) < line_width)
    //     drawline();

    // < 0.4
    // > 0.3
    float size = sprite_size * 0.35;
    vec2 uvf = vec2(mod(uv.x, size), mod(uv.y, size));
    if(uvf.x < line_width || uvf.y < line_width)
        drawline();

    // fragColor.rg = uvf;
    // fragColor.a = 1.0;
}

void main (void)
{

    vec4 pixel = texture(iChannel0, texCoord);
    fragColor = pixel;

    if((features & SFEATURE_BLOOM) != 0)
        bloom();

    if((features & SFEATURE_GRID) != 0)
        isometric_grid();

    // fragColor.rgb -= mod(gl_FragCoord.y, 2.0) < 1.0 ? 0.25 : 0.0;

    // if(abs(0.5-texCoord.x) < 0.001)
    //     fragColor = vec4(1.0);

    gl_FragDepth = 0;
}