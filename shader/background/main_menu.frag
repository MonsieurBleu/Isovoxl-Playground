#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

layout (location = 16) uniform sampler2D iChannel0;
layout (location = 17) uniform uint data;

in vec2 texCoord;

out vec4 fragColor;

float iTime = Time/20000.0;

//https://www.shadertoy.com/view/wscGWl

float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); } // random noise

float getCellBright(vec2 id) {
    return sin(((Time/2000.0)+2.0)*rand(id)*2.0)*0.5+0.5; // returns 0. to 1.
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

uint hue_preset[10] = uint[10](506, 462, 128, 347, 226, 264, 50, 351, 399, 220);

void main()
{
	float mx = max(float(win_const.x), float(win_const.y));
    vec2 uv = gl_FragCoord.xy / mx;
    
    uv.x += cos(iTime/8.0) - sin(iTime/8.0);
    uv.y += cos(iTime/8.0) + sin(iTime/8.0);

    // uv.x += cos(sqrt(iTime));
    // uv.y += sin(sqrt(iTime));
    
    uv.x = uv.x/2;

    // float angle = 1.5708;
    float angle = 0.785398;
    uv = vec2(uv.x*cos(angle)-uv.y*sin(angle), uv.x*sin(angle)+uv.y*cos(angle));


    float time = iTime;
    
    uv *= 90.0; // grid size

	vec2 id = floor(uv); // id numbers for each "cell"
    vec2 gv = fract(uv)-0.5; // uv within each cell, from -.5 to .5

	vec3 color = vec3(0.0);
    
	float randBright = getCellBright(id);

    vec3 colorShift = vec3(rand(id)*0.1); // subtle random color offset per "cell"
    
    // color = 0.6 + 0.5*cos(time + (id.xyx*0.1) + vec3(4,2,1) + colorShift); // RGB with color offset
    // color.r *= 0.25;
    // color.g *= 0.25;

    // color = 0.6 + 0.5*cos(time + (id.xyx*0.1) + vec3(4,2,1) + colorShift); // RGB with color offset
    
    // float test = (log2(id.x+1.0) + log2(id.y+1.0) + 1.0);

    // float test = 1.0 + sin(uv.x) + sin(uv.y);

    // float hueshift = 1.0 + id.x + id.y;
    // float satshift = 1.0 - id.x + id.y;

    // float hueshift = 1.0 + sin(id.x) - cos(id.y);
    // float satshift = 1.0 - cos(id.x) + sin(id.y);

    // float hueshift = 1.0 + log(id.x+1.0) + log(id.y+1.0);
    // float satshift = 1.0 - log(id.x+1.0) - log(id.y+1.0);

    // float hueshift = 0.05*id.x*id.x/(id.y+5.0); // cool!
    float hueshift = 1.0 - sin(id.x*0.15) + cos(id.y*0.15);
    float satshift = 1.0 + cos(-id.x*0.15) - sin(-id.y*0.15);;

    color.r = 0.50 + abs(cos(time + hueshift*0.25 + colorShift.x))*0.055; // => ripples classes !
    // color.r = 0.555 + cos(time + hueshift*0.25 + colorShift.x)*0.055; //=> la meilleur pour le moment
    // color.r = 0.80 + cos(time + hueshift*0.25 + colorShift.x)*0.25; // version alternative
    // color.r = cos(iTime)+ cos(time + hueshift*0.25 + colorShift.x)*0.08; // shift rgb classe

    // color.r = float(data)/512.0;
    // color.r += (float(data)/512.854);
    // color.r = color.r - floor(color.r);

    uint randshift = hue_preset[int((data/512.0)*10.0)];
    color.r = mod(color.r + randshift/512.0, 1.0); 

    color.r += abs(cos(time + hueshift*0.25 + colorShift.x))*0.055;
    // color.r += abs(cos(time + hueshift*0.25 + colorShift.x))*0.1;

    color.g = 0.75 + cos(time + satshift*0.75 + colorShift.x)*0.05;
    color.b = 0.70;

    // 0.60 et 0.65

    color = hsv2rgb(color);

    // troll mais en vrai ça passe super bien
    // color.rg = uv/250;
    // color.rg *= 1 + uv/500;

    float shadow = 0.0;
    shadow += smoothstep(0.0, 0.7,  gv.x*min(0.0, (getCellBright(vec2(id.x-1.0, id.y)) - getCellBright(id)))); // left shadow
    shadow += smoothstep(0.0, 0.7, -gv.y*min(0.0, (getCellBright(vec2(id.x, id.y+1.0)) - getCellBright(id)))); // top shadow
    
    color -= shadow*0.4;
    
    color *= 1.0 - (randBright*0.2);
    
	fragColor = vec4(color, 1.0);


    // fragColor.rgb -= mod(fragCoord.y*float(win_const.y), 2.0)<1.0 ? 0.5 : 0.0;
}