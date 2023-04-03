#ifndef TEXT_ENG_HPP
#define TEXT_ENG_HPP

#include <iostream>

const std::string TITLE = "ISOVOXL PLAYGROUND";
const std::string VERSION = "beta 0.17.3";

const std::string SETTINGS_FILENAME = "saves/settings.bin";

const std::string TXT_MAIN_MENU_QUIT = "QUIT";
const std::string TXT_MAIN_MENU_PLAY = "BUILD";
const std::string TXT_MAIN_MENU_CREDITS = "CREDITS";
const std::string TXT_MAIN_MENU_OPTIONS = "OPTIONS";

const std::string TXT_OPTIONS_VOLUME = "VOLUME";
const std::string TXT_OPTIONS_FULLSCREEN = "FULLSCREEN";
const std::string TXT_OPTIONS_RESOLUTION = "RESOLUTION";
const std::string TXT_OPTIONS_RESOLUTION_FLLSCR = "FULLSCREEN";
const std::string TXT_OPTIONS_RESOLUTION_BORLSS = "BORDERLESS";
const std::string TXT_OPTIONS_VSYNC = "VSYNC";
const std::string TXT_OPTIONS_SHADOWS = "SHADOWS";
const std::string TXT_OPTIONS_BLOCKS_BORDER = "BLOCKS BORDER";
const std::string TXT_OPTIONS_AO = "AMBIANT OCLUSION";
const std::string TXT_OPTIONS_CONTROLS = "CONTROLS";

const std::string TXT_OPTIONS_MUSICS = "MUSICS";
const std::string TXT_OPTIONS_SOUNDS = "SOUNDS";

const std::string IDMENU_UNTITLED = "untitled";
const std::string IDMENU_QUIT    = "Q";
const std::string IDMENU_PLAY    = "P";
const std::string IDMENU_CREDITS = "C";
const std::string IDMENU_OPTIONS = "O";

const std::string IDMENU_OPTIONS_VOLUME = "OVOL";
const std::string IDMENU_OPTIONS_FULLSCREEN = "OF";
const std::string IDMENU_OPTIONS_RESOLUTION = "ORES";
const std::string IDMENU_OPTIONS_RESOLUTION_FLLSCR = "ORESF";
const std::string IDMENU_OPTIONS_RESOLUTION_BORLSS = "ORESB";
const std::string IDMENU_OPTIONS_RESOLUTION_CUSTOM = "ORESC";

const std::string IDMENU_OPTIONS_RESOLUTION_CUSTOM_NEXT = "ORESCN";
const std::string IDMENU_OPTIONS_RESOLUTION_CUSTOM_PREV = "ORESCP";

const std::string IDMENU_OPTIONS_VSYNC = "OVS";
const std::string IDMENU_OPTIONS_SHADOWS = "OS";
const std::string IDMENU_OPTIONS_BLOCKS_BORDER = "OBB";
const std::string IDMENU_OPTIONS_AO = "OAO";

const std::string IDMENU_OPTIONS_CONTROLS = "OC";

const std::string IDMENU_WORLD_SELECTION_NEWMAP = "WNM";

const std::string WORLD_SELECTION_PLAY = "Play";
const std::string WORLD_SELECTION_NEWMAP = "New";    // -- seemed more natural to me
const std::string WORLD_SELECTION_DELMAP = "Delete"; // -- seemed more natural to me
const std::string WORLD_SELECTION_DELMAPSEC = "ARE YOU SURE";

const std::string NW_BIOMES_NAMES[9] = { "PLAINS    ",   
                                         "DUNES     ", 
                                         "SNOWVALLEY",
                                         "MOUNTAINS ",   
                                         "CANYON    ", 
                                         "ICEPEAKS  ",
                                         "DARK      ",   
                                         "FANTASY   ", 
                                         "FLAT      "};

const std::string NW_PRESET_NAMES[4] = { "PRESET 1", "PRESET 2", "PRESET 3", "PRESET 4"};

const std::string NW_SIZE_NAMES[5] = { "HUGE", "LARGE", "NORMAL", "SMALL", "TINY"};

const std::string NW_default_name = "";

const std::string NW_generating = "Generating Map...";
const std::string NW_name_exist = "This Map Name Arleady Exists";
const std::string NW_save = "SAVE MAP";

const std::string MeteoNames[] = 
{
    "Sky Blue",
    "Dusk",
    "Nebula",
    "Azur Aurora",
    "Orchid Aurora",
    "Scarlet Aurora",
    "Jade Aurora"
};

const std::string Tutorial[] = 
{
    "Right Click and drag to move the camera", // -- clarity
    "Left Click to place a block",
    "Mouse Wheel to zoom in/out", // -- clarity
    // "Mouse Wheel and L/R Shift to hide upper voxels",
    "Mouse Wheel and Shift to change the maximum view height",         // -- clarity
    "Mouse Wheel and CTRL to change the height of wall/volume tools",  // -- clarity
    " to rotate the camera",
    " to switch between construction modes",
    " to open block selection screen",
    " to toggle HUD",
    " to toggle grid",
    " to quicksave",
    " to quickload",
    "CTRL and Z/Y to undo/redo modifications",
    "Oh, and have fun !"
};

const std::string Mask_Mode_Indication = "Toggle replacing terrain when building";
const std::string Meteo_Indication = "Change Weather";
const std::string Help_Indication = "Help";

const std::string ALERT_Lights = " ! YOU HAVE REACHED THE MAXIMUM NUMBER OF LIGHT SOURCES ! ";

#endif