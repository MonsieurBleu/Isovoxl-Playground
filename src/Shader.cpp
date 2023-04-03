/*
Shader loading with uniform location caching and modern GLSL agnosticism.
By Jonathan Dearborn 2016

MIT License
*/

#include <Shader.hpp>
#include <cstring>
#include <cstdlib>

Shader::Shader()
    : program(0), vert(0), frag(0)
{}

Shader::~Shader()
{
    reset();
}


// Loads a shader and prepends version/compatibility info before compiling it.
// Normally, you can just use GPU_LoadShader() for shader source files or GPU_CompileShader() for strings.
// However, some hardware (certain ATI/AMD cards) does not let you put non-#version preprocessing at the top of the file.
// Therefore, I need to prepend the version info here so I can support both GLSL and GLSLES with one shader file.
// This function requires GLSL 130+.
/*
Do not use deprecated GLSL:
attribute, varying, texture2D, gl_FragColor
*/
static Uint32 load_shader_unversioned(GPU_ShaderEnum shader_type, const char* filename)
{
    SDL_RWops* rwops;
    Uint32 shader;
    char* source;
    int header_size, file_size;
    const char* header = "";
    GPU_Renderer* renderer = GPU_GetCurrentRenderer();
    
    // Open file
    rwops = SDL_RWFromFile(filename, "rb");
    if(rwops == NULL)
    {
        GPU_PushErrorCode("load_shader", GPU_ERROR_FILE_NOT_FOUND, "Shader file \"%s\" not found", filename);
        return 0;
    }
    
    // Get file size
    file_size = SDL_RWseek(rwops, 0, SEEK_END);
    SDL_RWseek(rwops, 0, SEEK_SET);
    
    // Get size from header
    if(renderer->shader_language == GPU_LANGUAGE_GLSL)
    {
        if(renderer->max_shader_version >= 330)
            header = "#version 330\n";
        else
            header = "#version 130\n";
    }
    else if(renderer->shader_language == GPU_LANGUAGE_GLSLES)
    {
        if(shader_type == GPU_FRAGMENT_SHADER)
        {
            header = "#version 100\n\
                     #ifdef GL_FRAGMENT_PRECISION_HIGH\n\
                     precision highp float;\n\
                     #else\n\
                     precision mediump float;\n\
                     #endif\n\
                     precision mediump int;\n";
        }
        else
            header = "#version 100\nprecision highp float;\nprecision mediump int;\n";
    }
    
    header_size = strlen(header);
    
    // Allocate source buffer
    source = (char*)malloc(sizeof(char)*(header_size + file_size + 1));
    
    // Prepend header
    strcpy(source, header);
    
    // Read in source code
    SDL_RWread(rwops, source + strlen(source), 1, file_size);
    source[header_size + file_size] = '\0';
    
    // Compile the shader
    shader = GPU_CompileShader(shader_type, source);
    
    // Clean up
    free(source);
    SDL_RWclose(rwops);
    
    return shader;
}



bool Shader::load(const std::string& vert_filename, const std::string& frag_filename, const std::string* geom_filename)
{
    std::cout << "\033[0;31m";
    reset();

    vert = GPU_LoadShader(GPU_VERTEX_SHADER, vert_filename.c_str());
    if(!vert)
    {
        GPU_Log("Vertex shader failed to load: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(vert);
    }
    
    frag = GPU_LoadShader(GPU_FRAGMENT_SHADER, frag_filename.c_str());
    if(!frag)
    {
        GPU_Log("Fragment shader failed to load: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(frag);
    }
    
    geom = 0;

    if(geom_filename)
    {
        geom = GPU_LoadShader(GPU_GEOMETRY_SHADER, geom_filename->c_str());
        if(!geom)
        {
            GPU_Log("Fragment shader failed to load: %s\n", GPU_GetShaderMessage());
            GPU_FreeShader(geom);
            // GPU_FreeShader(frag);
            // GPU_FreeShader(vert);
            // return false;
        }
    }

    if(!vert || !frag)
        return false;
    
    if(geom)
    {
        Uint32 shaders[3] = {vert, geom, frag};
        program = GPU_LinkManyShaders(shaders, 3);
    }
    else
        program = GPU_LinkShaders(vert, frag);



    if(!program)
    {
        GPU_Log("Shader program failed to link: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(vert);
        GPU_FreeShader(frag);
        return false;
    }

    block = GPU_LoadShaderBlock(program, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
    std::cout << "\033[0;0m";
    return true;
}



bool Shader::load_unversioned(const std::string& vert_filename, const std::string& frag_filename)
{
    reset();

    vert = load_shader_unversioned(GPU_VERTEX_SHADER, vert_filename.c_str());
    if(!vert)
    {
        GPU_Log("Vertex shader failed to load: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(vert);
    }
    
    frag = load_shader_unversioned(GPU_FRAGMENT_SHADER, frag_filename.c_str());
    if(!frag)
    {
        GPU_Log("Fragment shader failed to load: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(frag);
    }
    
    if(!vert || !frag)
        return false;
    
    program = GPU_LinkShaders(vert, frag);
    if(!program)
    {
        GPU_Log("Shader program failed to link: %s\n", GPU_GetShaderMessage());
        GPU_FreeShader(vert);
        GPU_FreeShader(frag);
        return false;
    }

    block = GPU_LoadShaderBlock(program, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
    
    return true;
}

void Shader::reset()
{
    GPU_FreeShaderProgram(program);
    GPU_FreeShader(vert);
    GPU_FreeShader(frag);
    
    vert = 0;
    frag = 0;
    program = 0;
    uniforms.clear();
    
    block.position_loc = -1;
    block.texcoord_loc = -1;
    block.color_loc = -1;
    block.modelViewProjection_loc = -1;
}

int Shader::load_location(const std::string& name)
{
    int loc = GPU_GetUniformLocation(program, name.c_str());
    if(loc >= 0)
        uniforms.insert(std::make_pair(name, loc));
    return loc;
}


void Shader::activate()
{
    GPU_ActivateShaderProgram(program, &block);
}

void Shader::deactivate()
{
    GPU_ActivateShaderProgram(0, NULL);
}

int Shader::get_location(const std::string& name)
{
    auto e = uniforms.find(name);
    if(e != uniforms.end())
        return e->second;
    
    // Try loading it to see if it exists...
    return load_location(name);
}

