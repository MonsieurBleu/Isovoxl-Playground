#version 430

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 5) uniform ivec4 win_const;

layout (location = 16) uniform sampler2D iChannel0;
layout (location = 17) uniform uint data;

in vec2 texCoord;
out vec4 fragColor;

float iTime = Time/1000.0;

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

//https://www.shadertoy.com/view/lljGDt

float rayStrength(vec2 raySource, vec2 rayRefDirection, vec2 coord, float seedA, float seedB, float speed)
{
	vec2 sourceToCoord = coord - raySource;
	float cosAngle = dot(normalize(sourceToCoord), rayRefDirection);
	
	return clamp(
		(0.45 + 0.15 * sin(cosAngle * seedA + iTime * speed)) +
		(0.3 + 0.2 * cos(-cosAngle * seedB + iTime * speed)),
		0.0, 1.0) *
		clamp((win_const.x - length(sourceToCoord)) / win_const.x, 0.5, 1.0);
}

void main()
{
	vec2 uv = gl_FragCoord.xy;
	uv.y = 1.0 - uv.y;
	vec2 coord = vec2(gl_FragCoord.x*win_const.x, win_const.y*gl_FragCoord.y);
	
    vec4 pixel_rays;

	// Set the parameters of the sun rays
	vec2 rayPos1 = vec2(win_const.x * 0.7, win_const.y * -0.4);
	vec2 rayRefDir1 = normalize(vec2(1.0, -0.116));
	float raySeedA1 = 36.2214;
	float raySeedB1 = 21.11349;
	float raySpeed1 = 1.5;
	
	vec2 rayPos2 = vec2(win_const.x * 0.8, win_const.y * -0.6);
	vec2 rayRefDir2 = normalize(vec2(1.0, 0.241));
	const float raySeedA2 = 22.39910;
	const float raySeedB2 = 18.0234;
	const float raySpeed2 = 1.1;
	
	// Calculate the colour of the sun rays on the current fragment
	vec4 rays1 =
		vec4(1.0, 1.0, 1.0, 1.0) *
		rayStrength(rayPos1, rayRefDir1, coord, raySeedA1, raySeedB1, raySpeed1);
	 
	vec4 rays2 =
		vec4(1.0, 1.0, 1.0, 1.0) *
		rayStrength(rayPos2, rayRefDir2, coord, raySeedA2, raySeedB2, raySpeed2);
	
	pixel_rays = rays1 * 0.5 + rays2 * 0.4;
	
	// Attenuate brightness towards the bottom, simulating light-loss due to depth.
	// Give the whole thing a blue-green tinge as well.
	float brightness = 1.0 - (coord.y / win_const.y);
	pixel_rays.r *= 0.1 + (brightness * 0.8);
	pixel_rays.g *= 0.3 + (brightness * 0.6);
	pixel_rays.b *= 0.5 + (brightness * 0.5);
    

    fragColor = vec4(0.0);
    fragColor.rgb += pixel_rays.rgb*pixel_rays.a*0.05;

    fragColor.a = 1.0;

    fragColor = pixel_rays;
}
