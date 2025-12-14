#ifndef SCRIPTFILE_HPP
#define SCRIPTFILE_HPP


#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace fs = std::filesystem;

struct ScriptFile_s {

    const std::string RESET             = "\033[0m";
    const std::string NEOBLUE           = "\033[94m";
    const std::string NEOGREEN          = "\033[92m";
    const std::string SCRIPT_LABEL      = NEOGREEN + "Script: " + RESET;
    const std::string DATA_LABEL        = NEOGREEN + "Data: " + RESET;
    const std::string MAIN_SCRIPT       = "main.txt";
    const std::string RECURVSICE_ANCHOR = "Circular reference detected: ";
    const std::string DIRECTORY_NAME    = "scripts";
    const std::string UNEVEN_BAC        = "Warning: Uneven number of + and - commands detected\n";
    const std::string CANT_OPEN         = "main.txt file not found";
    const std::string FILE_EXTENSION    = ".txt";
    const std::string RUN_FUNCTION      = "run ";
    const char SEMICOLON                = ';';
    const std::string CORRUPTED_TOKEN   = "Invalid number in token: ";
    const char PLUS                     = '+';
    const char MINUS                    = '-';
    const std::string COMMA             = ", ";
    const char OPEN_SBRACKED            = '[';
    const char CLOSED_SBRACKED          = ']';
    const std::string TYPO_WARNING      = "Warning: Unknown action '";
    const std::string IN_FILE           = "' in file ";
    const std::string FILE_NOT_FOUND    = "File not Found";

    bool hasTypo                        = false;
    bool isSetUp                        = false;

    std::vector<std::string> tokens;
    std::unique_ptr<int[]> data         = nullptr;   // primitive buffer for converted ints
    size_t tokenCount                   = 0;         // number of ints in data[]
    int binaryActionsCount;


    inline static const std::unordered_map<std::string, int> ACTION_MAP = {

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

    // loading scripts across several files recursively

    void load () {

        binaryActionsCount = 0;
        hasTypo = false;
        isSetUp = false;
        tokens.clear();
        loadScript(MAIN_SCRIPT);

    }

    void loadScript(const std::string& filename) {

        std::unordered_set<std::string> loaded;
        fs::path scriptsDir = fs::current_path() / DIRECTORY_NAME;  // Always relative to working dir
        loadScript(scriptsDir / filename, loaded);

    }

        void loadScript(const fs::path& filePath, std::unordered_set<std::string>& loaded) {

        // detect circular includes (recursion stack)
        if (loaded.contains(filePath.string())) {
            std::cerr << RECURVSICE_ANCHOR << filePath << "\n";
            return;
        }

        std::ifstream file(filePath);

        // Checking if the txt file is openable
        if (!file.is_open()) {
            std::cerr << CANT_OPEN << filePath << "\n";
            return;
        }

        // mark as currently being processed (recursion stack)
        loaded.insert(filePath.string());
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string token;

            while (std::getline(ss, token, SEMICOLON)) {

                if (token.empty()) continue;

                // Trim whitespace
                while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.erase(token.begin());
                while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))  token.pop_back();

                // Check for run function to load sub-scripts
                if (token.rfind(RUN_FUNCTION, 0) == 0) {

                    std::string subName = token.substr(RUN_FUNCTION.size());

                    if (!subName.ends_with(FILE_EXTENSION)) subName += FILE_EXTENSION;

                    fs::path subPath = filePath.parent_path() / subName;
                    ScriptFile_s sub;
                    sub.loadScript(subPath, loaded);
                    tokens.insert(tokens.end(), sub.tokens.begin(), sub.tokens.end());

                } else {
                    // existing token handling (checks + pushing)
                    if (!token.empty() && (token[0] == '+' || token[0] == '-')) {

                        std::string actionName = token.substr(1);

                        if (ACTION_MAP.find(actionName) == ACTION_MAP.end()) {
                            std::cerr << TYPO_WARNING << token << IN_FILE << filePath << "\n";
                            hasTypo = true;
                        }

                    } else {

                        std::stringstream ssToken(token);
                        std::string key, valueStr;
                        ssToken >> key >> valueStr;

                        if (!key.empty() && ACTION_MAP.find(key) == ACTION_MAP.end()) {
                            std::cerr << TYPO_WARNING << key << IN_FILE << filePath << "\n";
                            hasTypo = true;
                        }

                    }

                    tokens.push_back(token);
                }

            }
        }

        // finished processing this file — remove from recursion stack so it can be included again later
        loaded.erase(filePath.string());
    }

    void convert() {

    std::vector<int> temp;
    temp.reserve(tokens.size() * 2);

    for (const auto& tokenRaw : tokens) {

        std::string token = tokenRaw;
        if (token.empty()) continue;

        if (token[0] == '+' || token[0] == '-') {

            // handle + or - commands
            bool isPress = (token[0] == '+');
            std::string actionName = token.substr(1);
            auto it = ACTION_MAP.find(actionName);

            if (it != ACTION_MAP.end()) {

                temp.push_back(it->second);
                // Determine what to put in the "value/press" slot

                if (it->second <= -300 && it->second > -400) {

                    // single-trigger command → no hold, put 0
                    temp.push_back(0);

                } else {

                    temp.push_back(isPress ? 1 : 0);
                    binaryActionsCount++;

                }

            }

        } else {

            // No + or - prefix
            std::stringstream ss(token);
            std::string key, valueStr;
            ss >> key >> valueStr;

            auto it = ACTION_MAP.find(key);

            if (it != ACTION_MAP.end()) {

                if (it->second <= -200 && it->second > -300) {

                    // value command → expects numeric value

                    try {

                        int value = std::stoi(valueStr);
                        temp.push_back(it->second);
                        temp.push_back(value);

                    } catch (...) { std::cerr << CORRUPTED_TOKEN << token << "\n"; }

                } else if (it->second <= -300 && it->second > -400) {

                    // single-trigger command → no + or -, just append 0
                    temp.push_back(it->second);
                    temp.push_back(0);

                } else if (it->second <= -100 && it->second > -200) {

                    // bool command without + or - → assume press? Or skip? Here, default to 0
                    temp.push_back(it->second);
                    temp.push_back(0);

                }

            } else {

                std::cerr << TYPO_WARNING << key << IN_FILE << "unknown\n";

            }

        }

    }

    // sanity check: must be even number of binary actions
    if (!(binaryActionsCount % 2 == 0)) {

        std::cout << UNEVEN_BAC;
        return;

    }

    // allocate primitive buffer
    tokenCount = temp.size();
    data = std::make_unique<int[]>(tokenCount);

    for (size_t i = 0; i < tokenCount; ++i) data[i] = temp[i];

    if (hasTypo) {

        tokens.clear();
        data.reset();
        tokenCount = 0;
        return;

    }
        isSetUp = true;
}


    void printScript() const {

        std::cout << SCRIPT_LABEL;
        for (const auto& t : tokens) std::cout << t << SEMICOLON;
        std::cout << "\n";

    }

    void printData() const {

        std::cout << DATA_LABEL;
        std::cout << OPEN_SBRACKED;

        for (size_t i = 0; i < tokenCount; ++i) {

            std::cout << data[i];
            if (i + 1 < tokenCount) std::cout << COMMA;

        }
        std::cout << CLOSED_SBRACKED << "\n";
    }

};


#endif //SCRIPTFILE_HPP