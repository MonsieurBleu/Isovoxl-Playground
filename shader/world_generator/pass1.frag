#version 430

#define PI 3.1415926535897932384626433832795

layout (location = 5) uniform ivec4 win_const;

in vec4 texColor;
in vec2 texCoord;
uniform sampler2D world;

out vec4 fragColor;

///// https://github.com/patriciogonzalezvivo/lygia


/*
original_author: [Ian McEwan, Ashima Arts]
description: Classic Perlin Noise with periodic variant https://github.com/ashima/webgl-noise
use: pnoise(<vec2|vec3|vec4> pos, <vec2|vec3|vec4> periodic)
license: |
    Copyright (C) 2011 Ashima Arts. All rights reserved.
    Copyright (C) 2011-2016 by Stefan Gustavson (Classic noise and others)
      Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of the GPUImage framework nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

vec4 mod289(const in vec4 x) { return x - floor(x * (1. / 289.)) * 289.; }
vec4 permute(const in vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
vec4 taylorInvSqrt(in vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
vec2  quintic(const in vec2 v)  { return v*v*v*(v*(v*6.0-15.0)+10.0); }

// Classic Perlin noise, periodic variant
float pnoise(in vec2 P, in vec2 rep) {
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
    Pi = mod289(Pi);        // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;

    vec4 i = permute(permute(ix) + iy);

    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;

    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);

    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;

    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));

    vec2 fade_xy = quintic(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}

///////////////////////



vec2 coord_roration(vec2 uv, vec2 rpoint, float angle)
{
    angle = angle * PI/180;
    return vec2(
           (uv.x-rpoint.x)*cos(angle)-(uv.y-rpoint.y)*sin(angle) + rpoint.x, 
           (uv.x-rpoint.x)*sin(angle)+(uv.y-rpoint.y)*cos(angle) + rpoint.y);
}

void main (void)
{
    uint seed = 466541;
    vec2 uv = gl_FragCoord.xy + seed;

    uv *= 0.005;

    // float octave1 = (pnoise(uv,      vec2(50000, 50000)) );
    // float octave2 = (pnoise(uv*2.0,  vec2(50000, 50000)) )/8.0;
    // float octave3 = (pnoise(uv*8.0, vec2(50000, 50000)) )/32.0;
    // float octave4 = (pnoise(uv*16.0, vec2(50000, 50000)) )/32.0;

    // float finaloctave = (octave1 + octave2 + octave3 + octave4) + 1.0;
    // fragColor.g = finaloctave;


    float octave1 = (pnoise(uv*128, vec2(50000, 50000)) + 0.5)*1.25;
    float octave2 = pnoise(uv*32, vec2(50000, 50000))*0.75;
    float octave3 = pnoise(uv*32, vec2(50000, 50000))*0.0;

    fragColor.g = octave1 + octave2 + octave3;

    fragColor.rb = vec2(0.0);
    fragColor.a = 1.0;
}