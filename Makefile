CC = g++
LOWRAMFLAG = 
CPPFLAGS = -Wall -Wno-class-memaccess
SDLFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_gpu -lSDL2_mixer -O3

OBJ = obj/undo.o obj/audio_engine.o icon/icon.o obj/physics.o obj/coords.o obj/main.o obj/game.o obj/game-input.o obj/game-framerate.o obj/texture.o obj/render_engine.o obj/render_engine_2.o obj/world.o obj/multithreaded_event_handler.o obj/Shader.o obj/projection_grid.o obj/UI_text.o obj/UI_tile.o obj/UI_engine.o obj/meteo.o obj/sprites.o obj/animations.o obj/Agent.o obj/light_handler.o obj/world_generator.o

INCLUDE = -Iinclude 
EXEC = iso.exe
EXEC_LOWRAM = iso_low_ram.exe
DEL_win = del /Q /F
DEL = rm -f # linux

default: $(EXEC)

run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LOWRAMFLAG) -o $(EXEC) $(SDLFLAGS)

install : $(EXEC)

obj/main.o : main.cpp
	$(CC) -c  $(LOWRAMFLAG) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

obj/%.o : src/%.cpp
	$(CC) -c $(LOWRAMFLAG) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

lowram : 
	@echo > /dev/null 
	$(eval LOWRAMFLAG := -DLOW_GEOMETRY_MOD -DLOW_RAM_MOD)
	$(eval EXEC := $(EXEC_LOWRAM))

.PHONY : clean install run default cleanwin winreinstall lowram

clean : 
	$(DEL) $(EXEC) obj/*.o

cleanwin : 
	$(DEL_win) $(EXEC) obj\*.o

winreinstall : cleanwin install

countlines :
	find ./ -type f \( -iname \*.cpp -o -iname \*.hpp -o -iname \*.frag -o -iname \*.vert -o -iname \*.geom -o -iname \*.py \) | sed 's/.*/"&"/' | xargs  wc -l