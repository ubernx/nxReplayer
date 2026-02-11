module;

#include <string>
#include <unordered_map>

export module OpCodeToString;

export inline const std::unordered_map<std::string, int> ACTION_MAP = {

    {"crouch",               -101},  // CTRL
    {"moveleft",             -102},  // A
    {"moveright",            -103},  // D
    {"forward",              -104},  // W
    {"back",                 -105},  // S
    {"walk",                 -106},  // Shift
    {"left",                 -107},
    {"right",                -108},
    {"up",                   -109},
    {"down",                 -110},
    {"leanleft",             -111},  // Q
    {"leanright",            -112},  // E
    {"fire",                 -113},  // Mouse1
    {"zoom",                 -114},  // Mouse2
    {"wait",                 -201},
    {"yawspeed" ,            -202},
    {"pitchspeed",           -203},
    {"inventory",            -301},  // I
    {"medkit",               -302},  // #
    {"bandage",              -303},  // +
    {"sprint",               -304},  // X
    {"drop",                 -305},  // G
    {"savegame",             -306},  // F6
    {"loadgame",             -307},  // F7
    {"jump",                 -308},  // Space
    {"knife",                -309},  // 1
    {"pistol",               -310},  // 2
    {"rifle",                -311},  // 3
    {"nade",                 -312},  // 4
    {"binoc",                -313},  // 5
    {"bolt",                 -314},  // 6
    {"reload",               -315},  // R
    {"type",                 -316},  // Z
    {"launcher",             -317},  // V
    {"use",                  -318},  // F
    {"pause",                -319},  // F12
    {"escape",               -320},  // ESC
    {"firemode",             -321},  // 0
    {"timeaccel",            -322},  // NUMPAD ASTERISK
    {"timedeccel",           -323}   // NUMPAD SLASH

};