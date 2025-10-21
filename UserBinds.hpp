#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <dinput.h>
#include <algorithm>
#include <unordered_set>


struct UserBinds_s {

    const std::string BINDS             = "binds.txt";
    const std::string CANTOPEN          = "File not found or file has no reading permission.\n";
    const std::string INVALID_BINDS     = "Invalid binds detected, loading defaults...\n";
    const std::string INVALID_FORMAT    = "Invalid bind format detected: Missing '=' in bind\n";
    const std::string INVALID_ORDER     = "Bind order mismatch: expected '";
    const std::string INVALID_ACTION    = "Invalid action: ";
    const std::string INVALID_KEY       = "Invalid key: ";
    const std::string DUPLICATE_KEY     = "Duplicate key binding: ";
    const std::string ALLOWED_DUPLICATE = "MOUSE";

    const int BINDS_COUNT           = 31;

    std::vector<std::string> binds;
    int setupBinds[62];
    bool validBinds;

    void setup() {
        validBinds = true;
        binds.clear();
        importBinds();

        if (validBinds) {
            verifyBinds();
        }

        if (validBinds) {
            loadBinds();
        } else {
            std::cerr << INVALID_BINDS;
            loadDefaultBinds();
        }
    }

    // Converting the vector to an odd even separated array
    void loadBinds() {

        int idx = 0;
        for (const auto& fullLine : binds) {
            auto pos = fullLine.find('=');
            std::string actionName = trim(fullLine.substr(0, pos));
            std::string keyName = trim(fullLine.substr(pos + 1));

            setupBinds[idx++] = ACTION_MAP.at(actionName);
            setupBinds[idx++] = KEY_MAP.at(keyName);
        }

    }

    void importBinds() {
        std::ifstream file(BINDS);

        if (!file.is_open()) {
            std::cerr << CANTOPEN;
            validBinds = false;
            return;
        }


        std::string line;
        int lineIdx = 0;
        while (std::getline(file, line)) {
            // Trim leading whitespace
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            // Skip empty lines and comments
            if (line.empty() || (line.size() >= 2 && line[0] == '/' && line[1] == '/')) {
                continue;
            }

            // Ensure the line uses correct format
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                std::cerr << INVALID_FORMAT << line << "\n";
                validBinds = false;
                file.close();
                return;
            }

            std::string action = trim(line.substr(0, pos));
            std::string key = trim(line.substr(pos + 1));

            // Enforce ordering
            if (lineIdx < requiredActions.size() && action != requiredActions[lineIdx]) {
                std::cerr << INVALID_ORDER << requiredActions[lineIdx]
                          << "', got '" << action << "'\n";
                validBinds = false;
                file.close();
                return;
            }

            // Validate action exists
            if (ACTION_MAP.find(action) == ACTION_MAP.end()) {
                std::cerr << INVALID_ACTION << action << "\n";
                validBinds = false;
                file.close();
                return;
            }

            // Validate key exists
            if (KEY_MAP.find(key) == KEY_MAP.end()) {
                std::cerr << INVALID_KEY << key << "\n";
                validBinds = false;
                file.close();
                return;
            }

            binds.push_back(line);
            ++lineIdx;
        }

        // Must be exactly 31 at the end
        if (binds.size() != BINDS_COUNT) {
            validBinds = false;
        }

        file.close();
    }

    void verifyBinds() {
        std::unordered_set<std::string> foundKeys;

        for (const auto& line : binds) {
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                validBinds = false;
                return;
            }

            std::string key = trim(line.substr(pos + 1));

            // Check for duplicate keys (exclude mouse since 1-5 do exist)
            if (key.find(ALLOWED_DUPLICATE) == std::string::npos && foundKeys.count(key)) {
                std::cerr << DUPLICATE_KEY << key << "\n";
                validBinds = false;
                return;
            }

            foundKeys.insert(key);
        }
    }

    // Load default binds if user configuration is invalid
    void loadDefaultBinds() {
        setupBinds[0]  = ACTION_MAP.at("crouch");     setupBinds[1]  = DIK_LCONTROL;
        setupBinds[2]  = ACTION_MAP.at("moveleft");   setupBinds[3]  = DIK_A;
        setupBinds[4]  = ACTION_MAP.at("moveright");  setupBinds[5]  = DIK_D;
        setupBinds[6]  = ACTION_MAP.at("forward");    setupBinds[7]  = DIK_W;
        setupBinds[8]  = ACTION_MAP.at("back");       setupBinds[9]  = DIK_S;
        setupBinds[10] = ACTION_MAP.at("walk");       setupBinds[11] = DIK_LSHIFT;
        setupBinds[12] = ACTION_MAP.at("leanleft");   setupBinds[13] = DIK_Q;
        setupBinds[14] = ACTION_MAP.at("leanright");  setupBinds[15] = DIK_E;
        setupBinds[16] = ACTION_MAP.at("fire");       setupBinds[17] = 0x100;
        setupBinds[18] = ACTION_MAP.at("zoom");       setupBinds[19] = 0x101;
        setupBinds[20] = ACTION_MAP.at("inventory");  setupBinds[21] = DIK_I;
        setupBinds[22] = ACTION_MAP.at("medkit");     setupBinds[23] = DIK_RBRACKET;
        setupBinds[24] = ACTION_MAP.at("bandage");    setupBinds[25] = DIK_EQUALS;
        setupBinds[26] = ACTION_MAP.at("sprint");     setupBinds[27] = DIK_X;
        setupBinds[28] = ACTION_MAP.at("drop");       setupBinds[29] = DIK_G;
        setupBinds[30] = ACTION_MAP.at("savegame");   setupBinds[31] = DIK_F6;
        setupBinds[32] = ACTION_MAP.at("loadgame");   setupBinds[33] = DIK_F7;
        setupBinds[34] = ACTION_MAP.at("jump");       setupBinds[35] = DIK_SPACE;
        setupBinds[36] = ACTION_MAP.at("knife");      setupBinds[37] = DIK_1;
        setupBinds[38] = ACTION_MAP.at("pistol");     setupBinds[39] = DIK_2;
        setupBinds[40] = ACTION_MAP.at("rifle");      setupBinds[41] = DIK_3;
        setupBinds[42] = ACTION_MAP.at("nade");       setupBinds[43] = DIK_4;
        setupBinds[44] = ACTION_MAP.at("binoc");      setupBinds[45] = DIK_5;
        setupBinds[46] = ACTION_MAP.at("bolt");       setupBinds[47] = DIK_6;
        setupBinds[48] = ACTION_MAP.at("reload");     setupBinds[49] = DIK_R;
        setupBinds[50] = ACTION_MAP.at("type");       setupBinds[51] = DIK_Y;
        setupBinds[52] = ACTION_MAP.at("launcher");   setupBinds[53] = DIK_V;
        setupBinds[54] = ACTION_MAP.at("use");        setupBinds[55] = DIK_F;
        setupBinds[56] = ACTION_MAP.at("pause");      setupBinds[57] = DIK_PAUSE;
        setupBinds[58] = ACTION_MAP.at("escape");     setupBinds[59] = DIK_ESCAPE;
        setupBinds[60] = ACTION_MAP.at("firemode");   setupBinds[61] = DIK_0;
    }

    std::vector<std::string> requiredActions = {
        "crouch", "moveleft", "moveright", "forward", "back", "walk",
        "leanleft", "leanright", "fire", "zoom", "inventory", "medkit",
        "bandage", "sprint", "drop", "savegame", "loadgame", "jump",
        "knife", "pistol", "rifle", "nade", "binoc", "bolt", "reload",
        "type", "launcher", "use", "pause", "escape", "firemode"
    };

    inline static const std::unordered_map<std::string, DWORD> KEY_MAP = {
        // Letters
        {"A", DIK_A}, {"B", DIK_B}, {"C", DIK_C}, {"D", DIK_D}, {"E", DIK_E},
        {"F", DIK_F}, {"G", DIK_G}, {"H", DIK_H}, {"I", DIK_I}, {"J", DIK_J},
        {"K", DIK_K}, {"L", DIK_L}, {"M", DIK_M}, {"N", DIK_N}, {"O", DIK_O},
        {"P", DIK_P}, {"Q", DIK_Q}, {"R", DIK_R}, {"S", DIK_S}, {"T", DIK_T},
        {"U", DIK_U}, {"V", DIK_V}, {"W", DIK_W}, {"X", DIK_X}, {"Y", DIK_Y},
        {"Z", DIK_Z},

        // Numbers
        {"0", DIK_0}, {"1", DIK_1}, {"2", DIK_2}, {"3", DIK_3}, {"4", DIK_4},
        {"5", DIK_5}, {"6", DIK_6}, {"7", DIK_7}, {"8", DIK_8}, {"9", DIK_9},

        // Function keys
        {"F1", DIK_F1}, {"F2", DIK_F2}, {"F3", DIK_F3}, {"F4", DIK_F4},
        {"F5", DIK_F5}, {"F6", DIK_F6}, {"F7", DIK_F7}, {"F8", DIK_F8},
        {"F9", DIK_F9}, {"F10", DIK_F10}, {"F11", DIK_F11}, {"F12", DIK_F12},

        // Modifiers
        {"SHIFT", DIK_LSHIFT}, {"CTRL", DIK_LCONTROL}, {"ALT", DIK_LMENU},
        {"CAPSLOCK", DIK_CAPSLOCK}, {"TAB", DIK_TAB}, {"SPACE", DIK_SPACE},

        // Navigation
        {"UP", DIK_UP}, {"DOWN", DIK_DOWN}, {"LEFT", DIK_LEFT}, {"RIGHT", DIK_RIGHT},
        {"HOME", DIK_HOME}, {"END", DIK_END}, {"PGUP", DIK_PGUP}, {"PGDN", DIK_PGDN},
        {"INSERT", DIK_INSERT}, {"DELETE", DIK_DELETE},

        // Symbols (main keyboard)
        {"-", DIK_MINUS}, {"EQUALS", DIK_EQUALS}, {"=", DIK_EQUALS},
        {"[", DIK_LBRACKET}, {"]", DIK_RBRACKET},
        {"\\", DIK_BACKSLASH}, {";", DIK_SEMICOLON}, {"'", DIK_APOSTROPHE},
        {",", DIK_COMMA}, {".", DIK_PERIOD}, {"/", DIK_SLASH},
        {"`", DIK_GRAVE},

        // Numpad
        {"NUM0", DIK_NUMPAD0}, {"NUM1", DIK_NUMPAD1}, {"NUM2", DIK_NUMPAD2},
        {"NUM3", DIK_NUMPAD3}, {"NUM4", DIK_NUMPAD4}, {"NUM5", DIK_NUMPAD5},
        {"NUM6", DIK_NUMPAD6}, {"NUM7", DIK_NUMPAD7}, {"NUM8", DIK_NUMPAD8},
        {"NUM9", DIK_NUMPAD9},
        {"NUMPLUS", DIK_NUMPADPLUS}, {"+", DIK_NUMPADPLUS},
        {"NUMMINUS", DIK_NUMPADMINUS},
        {"NUMDEL", DIK_NUMPADPERIOD},
        {"NUMENTER", DIK_NUMPADENTER},

        // Special
        {"ESC", DIK_ESCAPE}, {"BACKSPACE", DIK_BACK}, {"ENTER", DIK_RETURN},
        {"PRINTSCREEN", DIK_SYSRQ}, {"PAUSE", DIK_PAUSE}, {"MENU", DIK_APPS},

        // Mouse placeholders
        {"MOUSE1", 0x100}, {"MOUSE2", 0x101}, {"MOUSE3", 0x102},
        {"MOUSE4", 0x103}, {"MOUSE5", 0x104},
    };

    inline static const std::unordered_map<std::string, int> ACTION_MAP = {
        {"crouch",      -101}, {"moveleft",    -102}, {"moveright",   -103},
        {"forward",     -104}, {"back",        -105}, {"walk",        -106},
        {"left",        -107}, {"right",       -108}, {"up",          -109},
        {"down",        -110}, {"leanleft",    -111}, {"leanright",   -112},
        {"fire",        -113}, {"zoom",        -114}, {"wait",        -201},
        {"yawspeed",    -202}, {"pitchspeed",  -203}, {"inventory",   -301},
        {"medkit",      -302}, {"bandage",     -303}, {"sprint",      -304},
        {"drop",        -305}, {"savegame",    -306}, {"loadgame",    -307},
        {"jump",        -308}, {"knife",       -309}, {"pistol",      -310},
        {"rifle",       -311}, {"nade",        -312}, {"binoc",       -313},
        {"bolt",        -314}, {"reload",      -315}, {"type",        -316},
        {"launcher",    -317}, {"use",         -318}, {"pause",       -319},
        {"escape",      -320}, {"firemode",    -321}
    };

    std::string trim(const std::string& s) {
        std::string result = s;
        result.erase(result.begin(), std::find_if(result.begin(), result.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));

        result.erase(std::find_if(result.rbegin(), result.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());

        return result;
    }
};