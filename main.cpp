#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <windows.h>
#include <algorithm>
#include <cctype>
#include "ScriptFile.hpp"
#include "ScriptReplayer.hpp"

// Loading ANSI Colors

void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

const std::string RESET       = "\033[0m";
const std::string NEOBLUE     = "\033[94m";
const std::string NEOCYAN     = "\033[96m";
const std::string UNLOADED_OR_UNCONVERTED = "The Script has not been loaded\n";
const std::string CMD         = "cmd: ";
const std::string UNKNOWN_CMD = "Unknown command: ";
const std::string MAIN_MENU   =
    NEOBLUE + "\nnxReplayer" + RESET + ", a TAS Emulator for S.T.A.L.K.E.R. Shadow Of Chernobyl\n"
    "_________________________________________________________________________________________________\n\n" +
    NEOBLUE + "Setup:" + RESET + " Place a 'scripts' folder next to the executable containing 'main.txt' as entry point.\n\n"
    "Following " + NEOBLUE + "Commands" + RESET + " are accessible for call:\n"
    "\n"
    "'" + NEOBLUE + "load" + RESET + "'            (loads the script from ./scripts/main.txt)\n"
    "'" + NEOBLUE + "script" + RESET + "'          (prints the loaded script to the console unconverted)\n"
    "'" + NEOBLUE + "data" + RESET + "'            (prints the integer array representation of the loaded script)\n"
    "'" + NEOBLUE + "run" + RESET + "'             (runs main.txt, calls load before executing script)\n"
    "'" + NEOBLUE + "start" + RESET + "'           (same a run, but only starts executing main.txt once client sync is passed)\n"
    "'" + NEOBLUE + "menu" + RESET + "'            (prints the main menu, what you currently see)\n"
    "'" + NEOBLUE + "exit" + RESET + "'            (closes and exits the program)\n\n"
    "Script Syntax: \n\n"

    "Binary Actions (use +/- prefix)     Trigger Actions (no prefix)       Functions (requires value)\n"
    "_________________________________________________________________________________________________\n"   + NEOCYAN +
    "crouch                              inventory                         wait <ms>\n"
    "moveleft                            medkit                            yawspeed <value>\n"
    "moveright                           bandage                           pitchspeed <value>\n"
    "forward                             sprint                            run <fileName.txt>\n"
    "back                                drop\n"
    "walk                                savegame\n"
    "left                                loadgame\n"
    "right                               jump\n"
    "up                                  knife\n"
    "down                                pistol\n"
    "leanleft                            rifle\n"
    "leanright                           nade\n"
    "fire                                binoc\n"
    "zoom                                bolt\n"
    "                                    reload\n"
    "                                    type\n"
    "                                    launcher\n"
    "                                    use\n"
    "                                    pause\n"
    "                                    escape\n"
    "                                    firemode\n"
    "                                    timeaccel\n"
    "                                    timedeccel\n"                                               + RESET +
        "_________________________________________________________________________________________________\n\n"
    "Example: +forward; wait 100; -forward; jump; +left; yawspeed 500; wait 200; -left; run backstrafe\n\n";


namespace fs = std::filesystem;

enum Command {

    CMD_LOAD,
    CMD_SCRIPT,
    CMD_DATA,
    CMD_RUN_SCRIPT,
    CMD_START_SCRIPT,
    CMD_MENU,
    CMD_EXIT,
    CMD_UNKNOWN 

};

std::unordered_map<std::string, Command> commandMap = {

    {"load",        CMD_LOAD        },
    {"script",      CMD_SCRIPT      },
    {"data",        CMD_DATA        },
    {"run",         CMD_RUN_SCRIPT  },
    {"start",       CMD_START_SCRIPT},
    {"menu",        CMD_MENU        },
    {"exit",        CMD_EXIT        }


};


std::string toLowerCase(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}


// Kernel

int main(int argc, char* argv[]) {

    enableANSI();
    ScriptFile_s script;
    ScriptReplayer_s replayer;
    std::cout << MAIN_MENU;


    for (;;) {

        std::cout << NEOBLUE << CMD << RESET;
        std::string userInput;
        std::cin >> userInput;
        userInput = toLowerCase(userInput);

        Command cmd = CMD_UNKNOWN;
        auto it = commandMap.find(userInput);
        if (it != commandMap.end()) cmd = it->second;


        switch (cmd) {

            case CMD_LOAD:        script.load();
                                  script.convert();
                break;


            case CMD_SCRIPT:      script.printScript();
                break;

            case CMD_DATA:        script.printData();
                break;

            case CMD_RUN_SCRIPT: {

                script.load();
                script.convert();

                if (script.isSetUp) {
                    replayer.run(script.data.get(), script.tokenCount);
                } else {
                    std::cout << UNLOADED_OR_UNCONVERTED;
                }
                break;
            }

            case CMD_START_SCRIPT: {
                script.load();
                script.convert();

                if (script.isSetUp) {

                    replayer.start(script.data.get(), script.tokenCount);
                } else {
                    std::cout << UNLOADED_OR_UNCONVERTED;
                }
                break;
            }

            case CMD_MENU: std::cout << MAIN_MENU;
                break;

            case CMD_EXIT:        return 0;

            default:              std::cout << UNKNOWN_CMD << userInput << '\n';

        }

    }

}