module;

#include <windows.h>
#include <string>
#include <unordered_map>
#include <iostream>

export module ScriptReplayer;

import InputSimulator;
import GameProcessUtils;
import UserBinds;

export struct ScriptReplayer_s {

    const std::string UNKNOWN_BINARY_ACTION        = "Unknown toggle (+/-)action: ";
    const std::string UNKNOWN_PARAMETERIZED_ACTION = "Unknown parameter: ";
    const std::string UNKNOWN_TRIGGER_ACTION       = "Unknown trigger action: ";
    const std::string SCRIPT_EXECUTION_TERMINATED  = "Script execution terminated by engaging ingame console\n";
    const std::string WAITING_FOR_CLIENT_SYNC      = "Waiting for client sync loading stage...\n";
    const std::string CLIENT_SYNC_DETECTED         = "Client Sync Detected!\n";
    const std::string CLIENT_INPUT_UNLOCKED        = "User Input Unlocked, starting script execution......\n";
    const std::string GAME_NOT_FOUND               = "Game process (XR_3DA.exe) or window not found\n";
    const std::string FAILED_OPEN_PROCESS          = "Failed to open process for memory reading\n";
    const std::string COULD_NOT_RESOLVE_MODULES    = "Could not resolve required module base addresses\n";
    const std::string PROCESS_NOT_FOUND_PREFIX     = "Process ";
    const std::string PROCESS_NOT_FOUND_SUFFIX     = " not found\n";
    const std::string NO_VISIBLE_WINDOW_PREFIX     = "No visible window found for process ";

    int* scriptData     = nullptr;
    size_t scriptLength = 0;

    float yawspeed    = 0.0f;
    float pitchspeed  = 0.0f;
    float accumMouseX = 0.0f;
    float accumMouseY = 0.0f;

    std::unordered_map<int, bool> heldStates; // allows binary actions to be togglable/detogglable, a togglemap

    GameProcessUtils_s  gameProcess;
    UserBinds_s         userBinds;
    InputSimulator      sim;

    enum BINARY_STATE_ACTIONS {

        CROUCH      = -101,
        MOVELEFT    = -102,
        MOVERIGHT   = -103,
        FORWARD     = -104,
        BACK        = -105,
        WALK        = -106,
        LEFT        = -107,
        RIGHT       = -108,
        UP          = -109,
        DOWN        = -110,
        LEANLEFT    = -111,
        LEANRIGHT   = -112,
        FIRE        = -113,
        ZOOM        = -114

    };

    enum PARAMETERIZED_STATE_ACTIONS {

        WAIT        = -201,
        YAWSPEED    = -202,
        PITCHSPEED  = -203

    };

    enum TRIGGER_STATE_ACTIONS {

        INVENTORY   = -301,
        MEDKIT      = -302,
        BANDAGE     = -303,
        SPRINT      = -304,
        DROP        = -305,
        SAVEGAME    = -306,
        LOADGAME    = -307,
        JUMP        = -308,
        KNIFE       = -309,
        PISTOL      = -310,
        RIFLE       = -311,
        NADE        = -312,
        BINOC       = -313,
        BOLT        = -314,
        RELOAD      = -315,
        TYPE        = -316,
        LAUNCHER    = -317,
        USE         = -318,
        PAUSE       = -319,
        ESCAPE      = -320,
        FIREMODE    = -321,
        TIMEACCEL   = -322,
        TIMEDECCEL  = -323

    };

    void start(int* data, size_t length) {

        scriptData   = data;
        scriptLength = length;
        yawspeed     = 0.0f;
        pitchspeed   = 0.0f;
        accumMouseX  = 0.0f;
        accumMouseY  = 0.0f;

        heldStates.clear();
        userBinds.setup();

        // Drop Execution if the game isn't running
        HWND hwnd = gameProcess.FindGameWindow("XR_3DA.exe");

        if (hwnd) {

            ShowWindow(hwnd, SW_RESTORE);  // Restore if minimized
            SetForegroundWindow(hwnd);

        } else {

            std::cerr << GAME_NOT_FOUND;
            return;  // Exit to avoid sending inputs elsewhere

        }

        DWORD pid = gameProcess.GetProcessIdFromWindow(hwnd);
        HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);

        if (!hProcess) {

            std::cerr << FAILED_OPEN_PROCESS;
            return;

        }

        // Getting what is being stored in gamestate addresses

        uintptr_t baseAddr = gameProcess.GetModuleBaseAddress(pid, "XR_3DA.exe");
        uintptr_t netBase  = gameProcess.GetModuleBaseAddress(pid, "xrNetServer.dll");

        if (!baseAddr || !netBase) {

            std::cerr << COULD_NOT_RESOLVE_MODULES;
            CloseHandle(hProcess);
            return;

        }

        // Memory Addresses which are used by STALKER to store the booting state of the game, 0000 v1.0000, 0006 v1.0006 shoc versions
        uintptr_t loadingAddr_0000 = netBase + 0xFAC4;
        uintptr_t loadingAddr_0006 = netBase + 0x13E84;
        BYTE loading_0000 = 0, loading_0006 = 0;
        std::cout << WAITING_FOR_CLIENT_SYNC;

        // Sleeppausing until game achieves client sync booting stage
        while (true) {

            ReadProcessMemory(hProcess, (LPCVOID)loadingAddr_0000, &loading_0000, 1, NULL);
            ReadProcessMemory(hProcess, (LPCVOID)loadingAddr_0006, &loading_0006, 1, NULL);
            if (loading_0000 == 1 || loading_0006 == 1) break;
            Sleep(1);

        }

        // Game is now in client sync, userinput is unlocked, initiate script
        std::cout << CLIENT_SYNC_DETECTED;
        while (true) { if (gameProcess.isSpinning1_0000(hProcess, baseAddr) || gameProcess.isSpinning1_0006(hProcess, baseAddr)) { break; } }

        // Game input is now unlocked, start script execution
        std::cout << CLIENT_INPUT_UNLOCKED;
        CloseHandle(hProcess);
        actionReader();

    }


    // Same as start yet without ingame client-sync-state checks and instant execution
    void run(int* data, size_t length) {

        scriptData      = data;
        scriptLength    = length;
        yawspeed        = 0.0f;
        pitchspeed      = 0.0f;
        accumMouseX     = 0.0f;
        accumMouseY     = 0.0f;

        heldStates.clear();
        userBinds.setup();
        actionReader();

    }

    void actionReader() {

        // Focus game window by process name
        HWND hwnd = gameProcess.FindGameWindow("XR_3DA.exe");

        if (hwnd) {

            ShowWindow(hwnd, SW_RESTORE);  // Restore if minimized
            SetForegroundWindow(hwnd);

        } else {

            std::cerr << GAME_NOT_FOUND;
            return;  // Exit to avoid sending inputs elsewhere

        }

        // *** CACHE ALL HANDLES/ADDRESSES ONCE BEFORE THE LOOP ***

        DWORD pid = gameProcess.GetProcessIdFromWindow(hwnd);
        HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);

        if (!hProcess) {

            std::cerr << FAILED_OPEN_PROCESS;
            return;

        }

        uintptr_t netBase = gameProcess.GetModuleBaseAddress(pid, "xrNetServer.dll");

        if (!netBase) {

            std::cerr << "Failed to get xrNetServer.dll base address.\n";
            CloseHandle(hProcess);
            return;

        }

        // Lambda to check game state using cached handles
        auto isGameLoaded = [&]() -> bool {

            BYTE loading_0000 = 0, loading_0006 = 0;
            ReadProcessMemory(hProcess, (LPCVOID)(netBase + 0xFAC4), &loading_0000, 1, NULL);
            ReadProcessMemory(hProcess, (LPCVOID)(netBase + 0x13E84), &loading_0006, 1, NULL);
            return (loading_0000 == 1 || loading_0006 == 1);

        };

        const float frameTime  = 0.002f;  // 2ms (500 FPS for safety)
        const DWORD frameSleep = 2;  // ms per frame

        for (size_t i = 0; i < scriptLength; i += 2) {

            int opcode = scriptData[i];
            int value = scriptData[i + 1];

            // Binary state actions (update state and send initial press/release)
            if (opcode >= -199 && opcode <= -100) {

                bool down = (value != 0);
                heldStates[opcode] = down;  // Track state

                switch (opcode) {

                    case CROUCH:     sim.SimulateKey(userBinds.setupBinds[1], down); break;
                    case MOVELEFT:   sim.SimulateKey(userBinds.setupBinds[3], down); break;
                    case MOVERIGHT:  sim.SimulateKey(userBinds.setupBinds[5], down); break;
                    case FORWARD:    sim.SimulateKey(userBinds.setupBinds[7], down); break;
                    case BACK:       sim.SimulateKey(userBinds.setupBinds[9], down); break;
                    case WALK:       sim.SimulateKey(userBinds.setupBinds[11], down); break;
                    case LEFT:                                                        break;
                    case RIGHT:                                                       break;
                    case UP:                                                          break;
                    case DOWN:                                                        break;
                    case LEANLEFT:   sim.SimulateKey(userBinds.setupBinds[13], down); break;
                    case LEANRIGHT:  sim.SimulateKey(userBinds.setupBinds[15], down); break;
                    case FIRE:       sim.SimulateMouseButton(userBinds.setupBinds[17], down); break;
                    case ZOOM:       sim.SimulateMouseButton(userBinds.setupBinds[19], down); break;
                    default:         std::cerr << UNKNOWN_BINARY_ACTION << opcode << "\n"; break;

                }
                // Parameterized state actions
            } else if (opcode >= -299 && opcode <= -200) {

                switch (opcode) {

                    case YAWSPEED:   yawspeed = static_cast<float>(value); break;
                    case PITCHSPEED: pitchspeed = static_cast<float>(value); break;
                    case WAIT: {

                        // Break wait into frames, applying held mouse looks each frame
                        for (DWORD elapsed = 0; elapsed < static_cast<DWORD>(value); elapsed += frameSleep) {
                            // Accumulate fractional mouse movement per frame (preserve precision)
                            float deltaX = 0.0f, deltaY = 0.0f;

                            if (heldStates[LEFT] && yawspeed != 0.0f)   { deltaX -= (yawspeed * frameTime); }
                            if (heldStates[RIGHT] && yawspeed != 0.0f)  { deltaX += (yawspeed * frameTime); }
                            if (heldStates[UP] && pitchspeed != 0.0f)   { deltaY -= (pitchspeed * frameTime); }
                            if (heldStates[DOWN] && pitchspeed != 0.0f) { deltaY += (pitchspeed * frameTime); }

                            // Add to accumulators (retain fractional parts)
                            accumMouseX += deltaX;
                            accumMouseY += deltaY;

                            // Only send integer movement; subtract sent amount from accumulators
                            int sendX = static_cast<int>(accumMouseX); // trunc toward zero
                            int sendY = static_cast<int>(accumMouseY);

                            if (sendX != 0 || sendY != 0) {

                                sim.SimulateMouseMove(sendX, sendY);
                                accumMouseX -= sendX;
                                accumMouseY -= sendY;

                            }
                            // Keys/buttons stay held automatically—no resend needed
                            sim.PreciseSleep(frameSleep);
                        }
                        break;
                    }
                    default: std::cerr << UNKNOWN_PARAMETERIZED_ACTION << opcode << "\n"; break;
                }
                // Trigger state actions
            } else if (opcode >= -399 && opcode <= -300) {

                switch (opcode) {

                    case INVENTORY:  TriggerKey(userBinds.setupBinds[21]); break;
                    case MEDKIT:     TriggerKey(userBinds.setupBinds[23]); break;
                    case BANDAGE:    TriggerKey(userBinds.setupBinds[25]); break;
                    case SPRINT:     TriggerKey(userBinds.setupBinds[27]); break;
                    case DROP:       TriggerKey(userBinds.setupBinds[29]); break;
                    case SAVEGAME:   TriggerKey(userBinds.setupBinds[31]); break;
                    case LOADGAME:   TriggerKey(userBinds.setupBinds[33]); break;
                    case JUMP:       TriggerKey(userBinds.setupBinds[35]); break;
                    case KNIFE:      TriggerKey(userBinds.setupBinds[37]); break;
                    case PISTOL:     TriggerKey(userBinds.setupBinds[39]); break;
                    case RIFLE:      TriggerKey(userBinds.setupBinds[41]); break;
                    case NADE:       TriggerKey(userBinds.setupBinds[43]); break;
                    case BINOC:      TriggerKey(userBinds.setupBinds[45]); break;
                    case BOLT:       TriggerKey(userBinds.setupBinds[47]); break;
                    case RELOAD:     TriggerKey(userBinds.setupBinds[49]); break;
                    case TYPE:       TriggerKey(userBinds.setupBinds[51]); break;
                    case LAUNCHER:   TriggerKey(userBinds.setupBinds[53]); break;
                    case USE:        TriggerKey(userBinds.setupBinds[55]); break;
                    case PAUSE:      TriggerKey(userBinds.setupBinds[57]); break;
                    case ESCAPE:     TriggerKey(userBinds.setupBinds[59]); break;
                    case FIREMODE:   TriggerKey(userBinds.setupBinds[61]); break;
                    case TIMEACCEL:  TriggerKey(VK_MULTIPLY); break;  // Numpad *

                    case TIMEDECCEL: {

                        INPUT input = {0};
                        input.type = INPUT_KEYBOARD;
                        input.ki.wVk = VK_DIVIDE;
                        input.ki.wScan = 0x35;
                        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
                        SendInput(1, &input, sizeof(INPUT));
                        sim.PreciseSleep(1);
                        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
                        SendInput(1, &input, sizeof(INPUT));
                        sim.PreciseSleep(1);
                        break;

                    }
                    default:         std::cerr << UNKNOWN_TRIGGER_ACTION << opcode << "\n"; break;
                }

            }

            if (!isGameLoaded()) {

                std::cerr << "Game is no longer loaded.\n";
                CloseHandle(hProcess);  // Clean up handle
                return;

            }

        }
        CloseHandle(hProcess);
    }

    void TriggerKey(WORD vkCode) {

        sim.SimulateKey(vkCode, true);
        sim.PreciseSleep(1);
        sim.SimulateKey(vkCode, false);
        sim.PreciseSleep(1);

    }

};

