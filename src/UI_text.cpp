#include <game.hpp>

unsigned char ASCII_to_ATLAS[255];

float debugratio = 1.0;

int iround(float x)
{
    return round(x);
}

void init_ASCII_to_ATLAS()
{
    for(int i = 0; i < 26; i++)
    {
        ASCII_to_ATLAS[i+'A'] = i;
        ASCII_to_ATLAS[i+'a'] = i+26;
    }

    for(int i = 0; i < 10; i++)
    {
        ASCII_to_ATLAS[i+'0'] = i+52;
    }

    ASCII_to_ATLAS['.']  = 62;
    ASCII_to_ATLAS[',']  = 63;
    ASCII_to_ATLAS['(']  = 64;
    ASCII_to_ATLAS[')']  = 65;
    ASCII_to_ATLAS['[']  = 66;
    ASCII_to_ATLAS[']']  = 67;
    ASCII_to_ATLAS['_']  = 68;
    ASCII_to_ATLAS['-']  = 69;
    ASCII_to_ATLAS['/']  = 70;
    ASCII_to_ATLAS['?']  = 71;
    ASCII_to_ATLAS['!']  = 72;
    ASCII_to_ATLAS['<']  = 73;
    ASCII_to_ATLAS['>']  = 74;
    ASCII_to_ATLAS['{']  = 75;
    ASCII_to_ATLAS['}']  = 76;
    ASCII_to_ATLAS[' ']  = 77;
}

UI_text::UI_text(
                std::shared_ptr<UI_tile> textfont,
                const std::string &strtext,
                int screenw,
                int screenh,
                float centered_x,
                float centered_y,
                float charsize, 
                Shader *textshdr,
                const std::string &textnameid,
                int textalign)
{
    font = textfont;

    // for(Uint64 i = 0; i < strtext.size(); i++)
    // {
    //     text.push_back(ASCII_to_ATLAS[(unsigned int)strtext[i]]);
    // }

    refresh_text(strtext);

    centered_posx = centered_x*screenw;
    centered_posy = centered_y*screenh;

    charwith = charsize*screenh;

    shader = textshdr;
    nameid = textnameid;

    align = textalign;
}

void UI_text::refresh_text(std::string const &strtext)
{
    textcpy = strtext;
    text.clear();

    for(Uint64 i = 0; i < strtext.size(); i++)
    {
        text.push_back(ASCII_to_ATLAS[(unsigned int)strtext[i]]);
    }
}

GPU_Rect UI_text::getRect()
{
    debugratio = (this->font->atlas->ptr->h/3.0)
                /(this->font->atlas->ptr->w/26.0);
    
    GPU_Rect r;

    r.w = charwith*(text.size() + 0.75);
    r.h = charwith*(debugratio  + 0.75);
    
    r.y = centered_posy;
    r.x = centered_posx;

    if(align == TEXT_ALIGN_LEFT)
    {
        r.x += 0.5*charwith*(float(text.size()) - 1.0);
    }
    if(align == TEXT_ALIGN_RIGHT)
    {
        r.x -= 0.5*charwith*(float(text.size()) - 1.0);
    }

    return r;
}

bool UI_text::render(GPU_Target *screen, Uint64 mod)
{
    debugratio = (this->font->atlas->ptr->h/3.0)
                /(this->font->atlas->ptr->w/26.0);

    bool hl = false;
    bool hl_letter = false;

    float hcharwith = charwith/2.0;

    float cpx  = centered_posx - hcharwith*(float(text.size()) - 1.0);
    float cpx2 = centered_posx + hcharwith*(float(text.size()) - 1.0);

    pixel_coord mouseUI;

    if(align == TEXT_ALIGN_LEFT)
    {
        cpx   = centered_posx;
        cpx2  = cpx + charwith*(float(text.size()) - 1.0);
    }
    if(align == TEXT_ALIGN_RIGHT)
    {
        cpx   = centered_posx - charwith*(float(text.size()) - 1.0);

        cpx2  = cpx + charwith*(float(text.size()) - 1.0);
    }

    font->change_size_norm(charwith, charwith);

    if(mod&TEXT_MOD_SHADER_ALL || mod&TEXT_MOD_SHADER_HL)
    {
        shader->activate();
        // int win_const[4] = {round(charwith*text.size()), screen->h, round(cpx), round(cpx2)};
        
        int win_const[4] = {iround(charwith*text.size()), 
                            iround(charwith), 
                            iround(cpx), 
                            iround(centered_posy-hcharwith)};

        GPU_SetUniformiv(5, 4, 1, win_const);
        GPU_SetUniformf(1, timems/1000.0);
    }

    if(mod&TEXT_MOD_SHADER_HL)
    {
        shader->deactivate();
    }

    if(mod&TEXT_MOD_HL_ALL)
    {
        mouseUI = {mouse.x*UI_SIZEDIFF, mouse.y*UI_SIZEDIFF};

        if(mouseUI.y > centered_posy-hcharwith && 
           mouseUI.y < centered_posy+hcharwith &&
           mouseUI.x > cpx -hcharwith &&
           mouseUI.x < cpx2+hcharwith
           )
           hl = true;
    }

    if(mod&TEXT_MOD_HL_FORCED)
        hl = true;

    int cpy2 = centered_posy;

    Uint64 ibeg = 0;
    Uint64 iend = text.size();

    // Uint64 val = floor((cos(timems*0.005f)+1.f)*2.f);
    // ibeg += val;
    // iend -= val;

    for(Uint64 i = ibeg; i < iend; i++)
    {
        font->change_atlas_id(text[i]);

        cpy2 = centered_posy;

        if(hl && mod&TEXT_MOD_HL_WAVE)
        {
            cpy2 += cos(timems*0.005 + i)*10.0 * screen->h/1080.f;
        }

        font->change_position_norm(cpx, cpy2);

        if(mod&TEXT_MOD_HL_PERLETTER && font->is_mouse_over())
        {
            // font->change_size_norm(charwith*1.25, charwith*1.25);
            // font->render(screen);
            // font->change_size_norm(charwith, charwith);

            hl_letter = true;
        }
        
        if(hl || hl_letter)
        {
            if(mod&TEXT_MOD_SHADER_HL)
                shader->activate();

            if(mod&TEXT_MOD_HL_BIGGER)
                font->change_size_norm(charwith*1.25, charwith*1.25*debugratio);
            else
                font->change_size_norm(charwith, charwith*debugratio);

            font->render(screen);
        }

        else
        {
            if(mod&TEXT_MOD_SHADER_HL)
                shader->deactivate();
                
            font->change_size_norm(charwith, charwith*debugratio);
            font->render(screen);
        }


        hl_letter = false;
        cpx += charwith;
    }

    shader->deactivate();

    return hl;
}