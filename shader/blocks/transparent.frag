#version 430

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8

#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32

layout (location = 1) uniform float Time;
layout (location = 2) uniform int features;
layout (location = 3) uniform vec3 light_direction;
layout (location = 4) uniform vec4 global_illumination;
layout (location = 5) uniform ivec4 win_const;
layout (location = 6) uniform float sprite_size;
layout (location = 7) uniform int block_size;
layout (location = 8) uniform int atlas_size;
layout (location = 9) uniform int max_height_render;
layout (location = 10) uniform int depth_mode;

uint render_flagsl;
uint render_flagsr;
uint render_flagst;
uint shadows_flags;
vec2 texCoord;

uint id;
uint block_height;
float PixelTint;
uint face;

in vec2 MozCoord;
in flat uint block_info;
in flat uint render_flags;

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D atlas;
layout (binding = 2) uniform sampler2D ao;
layout (binding = 3) uniform sampler2D DFIB;
layout (binding = 4) uniform sampler2D water;
layout (binding = 5) uniform sampler2D border;
layout (binding = 6) uniform sampler2D normal;
layout (binding = 7) uniform sampler2D light;
layout (binding = 8) uniform sampler2D world;
layout (binding = 9) uniform sampler2D hlworld;
layout (binding = 10) uniform sampler2D hlworld_depth;

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

float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); } // random noise


// https://thebookofshaders.com/11/
// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}


vec4 u_WaveStrengthX=vec4(4.15,4.66,0.0016,0.0015)*log(sprite_size)/6.0;
vec4 u_WaveStrengthY=vec4(2.54,6.33,0.00102,0.0025)*log(sprite_size)/6.0;
// vec4 u_WaveStrengthX=vec4(4.15,4.66,0.0016,0.0015) * 1/sprite_size * 18;
// vec4 u_WaveStrengthY=vec4(2.54,6.33,0.00102,0.0025) * 1/sprite_size * 18;
float iTime = Time;
vec2 dist(vec2 uv)
{ 
    float uTime = Time;
    if(uTime==0.0) uTime=0.15*iTime;
    float noise = texture(water, uTime + uv).r;
    uv.y += (cos((uv.y + uTime * u_WaveStrengthY.y + u_WaveStrengthY.x * noise)) * u_WaveStrengthY.z) +
        (cos((uv.y + uTime) * 10.0) * u_WaveStrengthY.w);

    uv.x += (sin((uv.y + uTime * u_WaveStrengthX.y + u_WaveStrengthX.x * noise)) * u_WaveStrengthX.z) +
        (sin((uv.y + uTime) * 15.0) * u_WaveStrengthX.w);
    return uv;
}

vec4 u_WaveStrengthX2=vec4(4.15,4.66,0.0016,0.0015)*log(sprite_size)/6.0;
vec4 u_WaveStrengthY2=vec4(2.54,6.33,0.00102,0.0025)*log(sprite_size)/6.0;
vec2 dist2(vec2 uv)
{ 
    float uTime = 151;
    float noise = texture(water, uTime + uv).r;
    uv.y += 2.0*( (cos((uv.y + uTime * u_WaveStrengthY2.y + u_WaveStrengthY2.x * noise)) * u_WaveStrengthY2.z) +
        (cos((uv.y + uTime) * 10.0) * u_WaveStrengthY2.w) );

    uv.x += 2.0*( (sin((uv.x + uTime * u_WaveStrengthX2.y + u_WaveStrengthX2.x * noise)) * u_WaveStrengthX2.z) +
        (sin((uv.x + uTime) * 15.0) * u_WaveStrengthX2.w) );
    return uv;
}

int extract_depth(vec4 DFIB)
{
    return int(DFIB.r*256.0) + (int(DFIB.g*256.0)/8)*256;
}

uint extract_face(vec4 DFIB)
{
    uint eface = (int(DFIB.g*256.0)<<30)>>30; // modulo didn't work here for some reasons

    if(eface != 0 && eface != 1)
        eface = 2;
    
    return eface;
}

void handle_highlight(vec4 pixel, vec4 pixel_norm, vec3 PixelTint)
{
    // vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    
    // vec4 DFIBp = texture(DFIB, screen_pos);

    // uint depth = extract_depth(DFIBp);

    // if(depth >= block_height) discard;
    
    // if((render_flagst&1) == 1)
    //     pixel.a *= 0.5;

    if(depth_mode > 0)
    {
        fragColor.r = (block_height%256)/256.0;
        fragColor.g = (block_height>>8)/256.0;
        fragColor.b = 0.0;
        fragColor.a = 1.0;
    }
    else
    {
        fragColor = pixel;
        fragColor.rgb*= PixelTint.rgb;
        fragColor.a = 0.85;
    }

    // fragColor.a = height;
}

void handle_water(vec4 pixel, vec4 pixel_norm)
{
    vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    vec2 screen_postmp = screen_pos;
    
    vec4 DFIBp = texture(DFIB, screen_pos);

    uint depth = extract_depth(DFIBp);

    if(depth >= block_height) discard;

    // determine depth height
    float water_depth;
    
    // vec4 pixel_world = texture(world, screen_pos);


    vec4 pixel_world;
    bool apply_hl = false;
    vec4 pixel_hl_depth = texture(hlworld_depth, screen_pos);
    int hl_depth = int(pixel_hl_depth.r*256.0) + (int(pixel_hl_depth.g*256.0)<<8);

    screen_pos = dist(screen_pos);
    if(hl_depth <= block_height)
    {
        if(hl_depth > 0)
        {
            pixel_world = texture(hlworld, screen_pos);
            if(pixel_world.a == 0.0)
                pixel_world = texture(world, screen_pos);
        }
        else
            pixel_world = texture(world, screen_pos);
    }
    else
    {
        apply_hl = true;
    }



    // depth = extract_depth(texture(DFIB, screen_pos));
    // if(depth >= block_height) depth = block_height;

    float max_water_depth = 16.0;
    // float max_water_depth = (cos(Time*2.0)+1.0)*16.0;
    uint tmp = (block_height-depth);
    // if(tmp%2 == 0) tmp -= 1;
    water_depth = float(tmp)/max_water_depth;

    if(water_depth < 0) water_depth = 0;
    if(water_depth > 2) water_depth = 2;

    water_depth = sqrt(water_depth);
    // water_depth = pow(water_depth, 0.3)*2.0;



    if(water_depth == 2)
    {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }


    pixel.g = pixel.g + ((2-water_depth)/16.0);

    /// water color desaturation
    float desaturation_water_depthue = ((1-water_depth)/6.0); // version colorée
    //float desaturation_water_depthue = ((2-water_depth)/4.0); // version réaliste
    vec3 pixel_hsv = rgb2hsv(pixel.rgb);
    pixel_hsv.y = pixel_hsv.y-desaturation_water_depthue;
    vec3 new_pixel = hsv2rgb(pixel_hsv.rgb);
    pixel.rgb = new_pixel.rgb;
    //////////////////////////////////////

    pixel.a += (water_depth/5.0); // := water opacity
    if(pixel.a > 1)
        pixel.a = 1;  


    pixel.rgb = rgb2hsv(pixel.rgb);
    pixel_world.rgb = rgb2hsv(pixel_world.rgb);

    // pixel_world.r = (pixel.r + pixel_world.r)/2;

    float color_shift_water_depthue = pixel_world.g*(2-water_depth)*(2-water_depth)/16.0;
    pixel.r = (pixel.r*pixel.a + pixel_world.r*color_shift_water_depthue)/(pixel.a+color_shift_water_depthue);
    // pixel.r = (pixel.r + pixel_world.r*color_shift_water_depthue)/(1+color_shift_water_depthue);

    // pixel_world.r = pixel.r*color_shift_water_depthue + pixel_world.r*(4-color_shift_water_depthue);

    pixel.rgb = hsv2rgb(pixel.rgb);
    pixel_world.rgb = hsv2rgb(pixel_world.rgb);

    fragColor.rgb = pixel.rgb*pixel.a + (pixel_world.rgb * (1-pixel.a));

    if(apply_hl)
    {
        pixel_world = texture(hlworld, screen_postmp);
        fragColor.rgb = pixel_world.rgb*pixel_world.a + (fragColor.rgb * (1-pixel_world.a));
    }

    fragColor.a = 1;
}

const float Persistance = 1;
const float Roughness = 0.75;

float diffraction(vec2 world_pos) {

    float diffraction_index = 0;
    float frequency = 0.5;
    float factor = 0.8;

    for (int i = 0; i < 4; i++) {
        diffraction_index += noise(world_pos * frequency / (sprite_size / 2048) * i * 0.72354) * factor;
        factor *= Persistance;
        frequency *= Roughness;
    }

    // diffraction_index = ((noise((world_pos * 0.1) / (sprite_size / 2048)) + noise((world_pos * 1) / (sprite_size / 2048)) *0.2 )) * 0.01; 
    return (1 - diffraction_index) / 20;
}

uint max_ice_depth = 9;
void handle_ice(vec4 pixel, vec4 pixel_norm)
{
    // vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    // screen_pos = dist2(screen_pos);
    // screen_pos = dist2(screen_pos);

    vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    vec2 screen_postmp = screen_pos;

    vec2 world_pos = vec2(
        screen_pos.x, 
        screen_pos.y
        ); 
    screen_pos = screen_pos + vec2(diffraction(world_pos)) * (sprite_size / 512.0 ) ;
    
    vec4 DFIBp = texture(DFIB, screen_pos);

    uint depth = extract_depth(DFIBp);

    uint ice_depth = (block_height-depth)+5;

    if(ice_depth > max_ice_depth) 
        ice_depth = max_ice_depth;
    
    pixel.a = sqrt(ice_depth)/sqrt(max_ice_depth);

    vec4 pixel_world;
    bool apply_hl = false;
    bool use_hl = false;
    vec4 pixel_hl_depth = texture(hlworld_depth, screen_postmp);
    int hl_depth = int(pixel_hl_depth.r*256.0) + (int(pixel_hl_depth.g*256.0)<<8);
    if(hl_depth < block_height)
    {
        if(hl_depth > 0)
        {
            use_hl = true;
        }
    }
    else
    {
        apply_hl = true;
    }

    if(pixel.a < 1.0)
    {
        vec3 color_buffer = vec3(0.0);
        vec2 screen_pos_buffer = vec2(texCoord + block_height);

        int max_pass = 1;

        for(int i = 0; i < max_pass; i++)
        {
            float xdec = rand(screen_pos_buffer)*0.0001*sprite_size; 
            float ydec = -rand(vec2(xdec))*0.0001*sprite_size; 

            int dirx = int(rand(vec2(xdec, ydec))*128.0);
            int diry = int(rand(vec2(ydec, xdec))*128.0);

            xdec *= dirx%2 == 0 ? -1 : 1;
            ydec *= diry%2 == 0 ? -1 : 1;

            // float xdec = noise(screen_pos_buffer)*0.0005*sprite_size; 
            // float ydec = -noise(vec2(xdec))*0.0005*sprite_size;

            screen_pos.x = screen_pos.x + xdec; 
            screen_pos.y = screen_pos.y + ydec; 

            float distblur = sqrt(xdec*xdec + ydec*ydec);

            if(use_hl)
            {
                vec4 colb = texture(hlworld, screen_pos);

                if(colb.a == 0.0)
                    colb = texture(world, screen_pos);

                color_buffer += colb.rgb * (1-distblur);
            }
            else
                color_buffer += texture(world, screen_pos).rgb * (1-distblur);

            screen_pos_buffer += rand(color_buffer.rg);
        }

        
        pixel_world.rgb = color_buffer.rgb/float(max_pass);
        pixel_world.a = 1.0;
    }

    pixel.r *= pixel.a;

    pixel.rgb = rgb2hsv(pixel.rgb);
    pixel.g *= 0.65;
    pixel.rgb = hsv2rgb(pixel.rgb);

    fragColor.rgb = (1-pixel.a)*pixel_world.rgb + pixel.a*pixel.rgb;

    if(apply_hl)
    {
        pixel_world = texture(hlworld, screen_postmp);
        fragColor.rgb = pixel_world.rgb*pixel_world.a + (fragColor.rgb * (1-pixel_world.a));
    }

    fragColor.a = 1.0;

    // fragColor.r *= pixel.a;
}


void handle_glass(vec4 pixel, vec4 pixel_norm)
{
    vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    vec2 screen_postmp = screen_pos;

    // vec4 DFIBp;
    // DFIBp = texture(DFIB, screen_pos);
    // int depth = extract_depth(DFIBp);
    // if(depth >= block_height) discard;

    vec3 color_buffer = vec3(0.0);
    vec2 screen_pos_buffer = vec2(texCoord + block_height);

    bool apply_hl = false;
    bool use_hl = false;
    vec4 pixel_hl_depth = texture(hlworld_depth, screen_postmp);
    int hl_depth = int(pixel_hl_depth.r*256.0) + (int(pixel_hl_depth.g*256.0)<<8);
    if(hl_depth < block_height)
    {
        if(hl_depth > 0)
        {
            use_hl = true;
        }
    }
    else
    {
        apply_hl = true;
    }

    int max_pass = 4;

    for(int i = 0; i < max_pass; i++)
    {
        float xdec = rand(screen_pos_buffer)*0.0001*sprite_size; 
        float ydec = -rand(vec2(xdec))*0.0001*sprite_size; 

        int dirx = int(rand(vec2(xdec, ydec))*128.0);
        int diry = int(rand(vec2(ydec, xdec))*128.0);

        xdec *= dirx%2 == 0 ? -1 : 1;
        ydec *= diry%2 == 0 ? -1 : 1;

        // float xdec = noise(screen_pos_buffer)*0.0005*sprite_size; 
        // float ydec = -noise(vec2(xdec))*0.0005*sprite_size;

        screen_pos.x = screen_pos.x + xdec; 
        screen_pos.y = screen_pos.y + ydec; 

        float distblur = sqrt(xdec*xdec + ydec*ydec);

        if(use_hl)
        {
            vec4 colb = texture(hlworld, screen_pos);

            if(colb.a == 0.0)
                colb = texture(world, screen_pos);

            color_buffer += colb.rgb * (1-distblur);
        }
        else
            color_buffer += texture(world, screen_pos).rgb * (1-distblur);

        screen_pos_buffer += rand(color_buffer.rg);
    }

    vec4 pixel_world;
    
    pixel_world.rgb = color_buffer.rgb/float(max_pass);
    pixel_world.a = 1.0;

    // test poussière
    // float test = rand(texCoord * block_height);
    // int testest = int(test*16000.0);
    // if(testest%105 == 0) pixel.a *= 2;

    fragColor.rgb = (1-pixel.a)*pixel_world.rgb + pixel.a*pixel.rgb;

    if(apply_hl)
    {
        pixel_world = texture(hlworld, screen_postmp);
        fragColor.rgb = pixel_world.rgb*pixel_world.a + (fragColor.rgb * (1-pixel_world.a));
    }

    fragColor.a = 1.0;
}

float Shadows(vec4 pixel_norm)
{
    if(
        pixel_norm.r == 1 && (shadows_flags&SHADOW_RIGHT) != 0 ||
        pixel_norm.g == 1 && (shadows_flags&SHADOW_LEFT)  != 0 ||
        pixel_norm.b == 1 && (shadows_flags&SHADOW_TOP)   != 0
    ){
        return 0.75;
    }
    return 1;
}

#define PI 3.1415926535897932384626433832795

vec2 coord_roration(vec2 uv, vec2 rpoint, float angle)
{
    angle = angle * PI/180;
    return vec2(
           (uv.x-rpoint.x)*cos(angle)-(uv.y-rpoint.y)*sin(angle) + rpoint.x, 
           (uv.x-rpoint.x)*sin(angle)+(uv.y-rpoint.y)*cos(angle) + rpoint.y);
}

void main (void)
{
//     fragColor = vec4(0.0);
//     fragColor.a = 1.0;
//     gl_FragDepth = 0.0;
    // return;

    id = block_info%256;

    vec4 pixel;

    // texCoord.x = float(int(MozCoord.x*atlas_size)%block_size)/block_size;
    // texCoord.y = float(int(MozCoord.y*atlas_size)%block_size)/block_size;

    texCoord = MozCoord;

    // id++;
    vec2 ColorCoord = vec2(float(id%16)/16.0, float(id/16)/16.0);
    pixel = texture(atlas, ColorCoord);
    if(id == 0)
    {
        pixel = vec4(1.0);
        pixel.a = 0.75;
    }

    vec4 pixel_norm = texture(normal, texCoord);

    if(pixel_norm.a == 0) discard; // discarding invisble fragments

    block_height = block_info>>8;
    /// CUSTOM DEPTH TESTING ///
    vec2 screen_pos = vec2(gl_FragCoord.x/win_const.x, (gl_FragCoord.y/win_const.y));
    vec4 DFIBp = texture(DFIB, screen_pos);
    uint depth = extract_depth(DFIBp);

    vec2 tc2 = texCoord;

    tc2 = coord_roration(tc2, vec2(0.5), 180);

    vec4 pixel_part = texture(normal, tc2);

    // fragColor = pixel_part;
    // return;


    render_flagsl = render_flags%256;
    render_flagsr = (render_flags>>8)%256;
    render_flagst = (render_flags>>16)%256;
    shadows_flags = (render_flags>>24)%256;

    if(id > 200)
    {
        if(pixel_part.b == 1.0)
        {
            if(depth >= block_height) discard;
        }
        else
        {
            if(depth > block_height) discard;
        }
    }
    else
    {
        if(pixel_part.b == 1.0 && (shadows_flags&1) == 1)
        {
            if(depth >= block_height) discard;
        }
        else
        {
            if(depth > block_height) discard;
        }
    }

    //////////////////////////////////

    vec4 pixel_AO = texture(ao, texCoord);

    //////////////////////// FACE CULLING /////////////////////////
    if(pixel_norm.g == 1 && render_flagsl < 128) discard;

    if(pixel_norm.r == 1 && render_flagsr < 128) discard;

    // if(block_height != max_height_render && id < 240 && pixel_norm.b == 1 && render_flagst < 128) discard;
    // if(id < 240 && pixel_norm.b == 1 && render_flagst < 128) discard;

    if((id < 240 || block_height != max_height_render) 
       && pixel_norm.b == 1 
       && render_flagst < 128) discard;
    ////////////////////////////////////////////////////////////////

    ///// GLOBAL ILLUMINATION /////
    float GI = dot(light_direction, pixel_norm.rgb);
    vec4 PixelTint = vec4(global_illumination.xyz, 1);
    // if(GI < 0.75)
    //     GI = 0.75;

    PixelTint *= GI;
    PixelTint.rgb *= Shadows(pixel_norm);

    gl_FragDepth = 1-(float(block_height)+1.0)/16384; // 2^14;

    if(id == 240)
    {
        handle_water(pixel, pixel_norm);
        fragColor.rgb*= PixelTint.rgb;
        return;
    }
    if(id == 241)
    {
        handle_ice(pixel, pixel_norm);
        fragColor.rgb*= PixelTint.rgb;
        return;
    }
    else if(id > 240)
    {
        handle_glass(pixel, pixel_norm);
        fragColor.rgb*= PixelTint.rgb;
        return;
    }
    else
    {
        handle_highlight(pixel, pixel_norm, PixelTint.rgb);
        // fragColor.rgb*= PixelTint.rgb;
        // gl_FragDepth = 0.0;
        return;
    }
}