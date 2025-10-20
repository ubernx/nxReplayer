#ifndef GAMEPROCESSUTILS_HPP
#define GAMEPROCESSUTILS_HPP

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <cstring> // _stricmp

struct GameProcessUtils_s {

    struct WindowData { DWORD pid; HWND hwnd; };

    HWND FindGameWindow(const char* processName) {
        DWORD pid = 0;
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

        if (data.hwnd == NULL) {
            std::cerr << "No visible window found for process " << processName << "\n";
        }

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

    // 1.0000
    bool IsLoadingScreenActive(HANDLE processHandle) {
        uintptr_t loadingScreenAddr = 0x004D0644;
        byte loadingScreen = 0;
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(loadingScreenAddr), &loadingScreen, 1, NULL);
        return (loadingScreen == 1);
    }

    // 1.0006
    bool IsGameLoading(HANDLE processHandle) {
        uintptr_t isLoadingAddr = 0x00465FC4;
        byte isLoading = 0;
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(isLoadingAddr), &isLoading, 1, NULL);
        return (isLoading == 1);
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
