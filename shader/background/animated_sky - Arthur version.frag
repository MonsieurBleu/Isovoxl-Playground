#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

in vec2 texCoord;
layout (location = 16) uniform sampler2D iChannel0;

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


vec2 uv_step(vec2 uv, float step) // creates a pixelated effect, step is the size of the pixels. uv is the texture coordinate (0, 0) to (1, 1)
{
    uv.x = floor((uv.x * win_const[0]) / step) * step ;
    uv.y = floor((uv.y * win_const[1]) / step) * step ;
    uv.x /= win_const[0];
    uv.y /= win_const[1];
    return uv;
}

vec2 uv_step(vec2 uv, float step, float offset) {
    uv.x = floor((uv.x * win_const[0]) / step) * step + offset;
    uv.y = floor((uv.y * win_const[1]) / step) * step + offset;
    uv.x /= win_const[0];
    uv.y /= win_const[1];
    return uv;
}



// https://www.shadertoy.com/view/XlsXDB

#define saturate(x) clamp(x,0.,1.)
#define rgb(r,g,b) (vec3(r,g,b)/255.)

float rand(float x) { return fract(sin(x) * 71.5413291); }

float rand(vec2 x) { return rand(dot(x, vec2(13.4251, 15.5128))); }

float noise(vec2 x)
{
    vec2 i = floor(x);
    vec2 f = x - i;
    f *= f*(3.-2.*f);
    return mix(mix(rand(i), rand(i+vec2(1,0)), f.x),
               mix(rand(i+vec2(0,1)), rand(i+vec2(1,1)), f.x), f.y);
}

float fbm(vec2 x)
{
    float r = 0.0, s = 1.0, w = 1.0;
    for (int i=0; i<5; i++)
    {
        s *= 2.0;
        w *= 0.5;
        r += w * noise(s * x);
    }
    return r;
}

float cloud(vec2 uv, float scalex, float scaley, float density, float sharpness, float speed)
{
    return pow(saturate(fbm(vec2(scalex,scaley)*(uv+vec2(speed,0)*(Time / 1000.0)))-(1.0-density)), 1.0-sharpness);
}

vec3 render(vec2 uv)
{
    uv = uv_step(uv, 8);
    // sky
    vec3 color = mix(rgb(255,212,166), rgb(204,235,255), uv.y);
    // sun
    vec2 spos = uv - vec2(1.1, 0.8);
    float sun = exp(-20.*dot(spos,spos));
    vec3 scol = rgb(255,155,102) * sun * 0.7;
    color += scol;
    // clouds
    vec3 cl1 = mix(rgb(151,138,153), rgb(166,191,224),uv.y);
    float d1 = mix(0.9,0.1,pow(uv.y, 0.7));
    color = mix(color, cl1, cloud(uv,2.,8.,d1,0.4,0.04));
    color = mix(color, vec3(0.9), 8.*cloud(uv,14.,18.,0.9,0.75,0.02) * cloud(uv,2.,5.,0.6,0.15,0.01)*uv.y);
    color = mix(color, vec3(0.8), 5.*cloud(uv,12.,15.,0.9,0.75,0.03) * cloud(uv,2.,8.,0.5,0.0,0.02)*uv.y);
    // post
    color *= vec3(1.0,0.93,0.81)*1.04;
    color = mix(0.75*rgb(255,205,161), color, smoothstep(-0.1,0.3,uv.y));
    color = pow(color,vec3(1.3));
    vec3 hsv = rgb2hsv(color);
    hsv.y *= 1.3;
    hsv.z *= 1.1;
    color = hsv2rgb(hsv);
    return color;
}

vec4 CloudySunset(vec2 fragCoord )
{
	vec2 uv = fragCoord.xy;
    
	return vec4(render(uv),1.0);
}

void main (void)
{
    //vec4 pixel = texture2D(iChannel0, texCoord);
    fragColor = CloudySunset(texCoord);
    vec3 v = vec3(1.0, 0.83, 0.65);
    gl_FragDepth = 0;

    // fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}