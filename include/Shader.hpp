/*
Shader loading with uniform location caching and modern GLSL agnosticism.
By Jonathan Dearborn 2016

MIT License
*/

#ifndef _SHADER_HPP__
#define _SHADER_HPP__

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>
#include <string>
#include <map>

class Shader
{
public:
    Uint32 program;
    
    Uint32 vert, frag, geom;
    
    GPU_ShaderBlock block;
    std::map<std::string, int> uniforms;
    
    
    Shader();
    ~Shader();
    
    bool load(const std::string& vert_filename, const std::string& frag_filename, const std::string* geom_filename);
    bool load_unversioned(const std::string& vert_filename, const std::string& frag_filename);
    void reset();
    
    int load_location(const std::string& name);
    
    void activate();
    void deactivate();
    int get_location(const std::string& name);
    
};

#endif
