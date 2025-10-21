#ifndef INPUTSIMULATOR_HPP
#define INPUTSIMULATOR_HPP

#include <windows.h>
#include <winuser.h>

struct InputSimulator {
private:
    HWND targetWindow = nullptr;

public:
    InputSimulator() {
        timeBeginPeriod(1);
    }

    ~InputSimulator() {
        timeEndPeriod(1);
    }

    void SetTargetWindow(HWND hwnd) {
        targetWindow = hwnd;
    }

    void SimulateKey(WORD vkCode, bool down) {
        // Use SendInput for key simulation
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkCode;
        input.ki.wScan = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
        // Extended key flag for certain keys
        if (vkCode == VK_CONTROL || vkCode == VK_SHIFT ||
            vkCode == VK_MENU || vkCode == VK_UP || vkCode == VK_DOWN ||
            vkCode == VK_LEFT || vkCode == VK_RIGHT ||
            (vkCode >= VK_F1 && vkCode <= VK_F24)) {
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }
        SendInput(1, &input, sizeof(INPUT));
        Sleep(1);
    }

    void SimulateMouseButton(int button, bool down) {
        // button: 1 = left, 2 = right
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        if (button == 1) {
            input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        } else {
            input.mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        }
        SendInput(1, &input, sizeof(INPUT));
    }

    void SimulateMouseMove(int dx, int dy) {
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

    void PreciseSleep(DWORD ms) {
        if (ms >= 2) {
            Sleep(ms);
        } else {
            LARGE_INTEGER freq, start, end;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start);
            end.QuadPart = start.QuadPart + (freq.QuadPart * ms / 1000);
            while (start.QuadPart < end.QuadPart) {
                QueryPerformanceCounter(&start);
            }
        }
    }
};

#endif // INPUTSIMULATOR_HPP