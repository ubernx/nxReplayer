#ifndef GAMEPROCESSUTILS_HPP
#define GAMEPROCESSUTILS_HPP

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <cstring> // _stricmp
#include <cmath>   // std::fabs
#include <cstdint> // uint8_t

struct GameProcessUtils_s {

    struct WindowData { DWORD pid; HWND hwnd; };

    HWND FindGameWindow(const char* processName) {

        DWORD pid       = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot != INVALID_HANDLE_VALUE) {

            PROCESSENTRY32 entry;
            entry.dwSize = sizeof(entry);

            if (Process32First(snapshot, &entry)) {

                do {

                    if (_stricmp(entry.szExeFile, processName) == 0) {

                        pid = entry.th32ProcessID;
                        break;

                    }

                } while (Process32Next(snapshot, &entry));

            }
            CloseHandle(snapshot);
        }

        if (pid == 0) {

            std::cerr << "Process " << processName << " not found\n";
            return NULL;

        }

        WindowData data = { pid, NULL };
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

        if (data.hwnd == NULL) { std::cerr << "No visible window found for process " << processName << "\n"; }

        return data.hwnd;
    }


    DWORD GetProcessIdFromWindow(HWND hwnd) {

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        return pid;

    }


    uintptr_t GetModuleBaseAddress(DWORD pid, const char* moduleName) {

        uintptr_t baseAddr = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

        if (snapshot != INVALID_HANDLE_VALUE) {

            MODULEENTRY32 entry;
            entry.dwSize = sizeof(entry);

            if (Module32First(snapshot, &entry)) {

                do {

                    if (_stricmp(entry.szModule, moduleName) == 0) {

                        baseAddr = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
                        break;

                    }

                } while (Module32Next(snapshot, &entry));

            }
            CloseHandle(snapshot);
        }
        return baseAddr;
    }

    // 1.0000 - now takes baseAddr parameter
    bool IsLoadingScreenActive(HANDLE processHandle, uintptr_t baseAddr) {

        uintptr_t loadingScreenAddr = baseAddr + 0xD0644;  // Offset from base
        byte loadingScreen = 0;
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(loadingScreenAddr), &loadingScreen, 1, NULL);
        return (loadingScreen == 1);

    }

    // 1.0000 - now takes baseAddr parameter
    bool isActorSpinning(HANDLE processHandle, uintptr_t baseAddr) {

        const uintptr_t yawAddr = baseAddr + 0x10498C;  // Offset from base
        float v1, v2;
        SIZE_T read = 0;
        const float eps = 1e-4f;

        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(yawAddr), &v1, sizeof(v1), &read);
        Sleep(1);
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(yawAddr), &v2, sizeof(v2), &read);
        return fabsf(v1 - v2) > eps;

    }

    // 1.0006 - now takes baseAddr parameter
    bool IsGameLoading(HANDLE processHandle, uintptr_t baseAddr) {

        uintptr_t isLoadingAddr = baseAddr + 0xD7B9C;  // Offset from base
        byte isLoading = 0;
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(isLoadingAddr), &isLoading, 1, NULL);
        return (isLoading == 1);

    }

    // 1.0006 - now takes baseAddr parameter
    bool isSpinning(HANDLE processHandle, uintptr_t baseAddr) {

        const uintptr_t yawAddr = baseAddr + 0x10BEE4;  // Offset from base
        float v1, v2;
        SIZE_T read = 0;
        const float eps = 1e-4f;

        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(yawAddr), &v1, sizeof(v1), &read);
        Sleep(1);
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(yawAddr), &v2, sizeof(v2), &read);
        return fabsf(v1 - v2) > eps;

    }


    private:
        // EnumWindows callback
        static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {

            DWORD windowPid;
            GetWindowThreadProcessId(hwnd, &windowPid);
            WindowData* data = reinterpret_cast<WindowData*>(lParam);

            if (windowPid == data->pid && IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {

                data->hwnd = hwnd;
                return FALSE;

            }
            return TRUE;
        }

};

#endif // GAMEPROCESSUTILS_HPP