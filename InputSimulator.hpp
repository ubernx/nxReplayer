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
        if (!targetWindow) {
            UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
            DWORD flags = down ? 0 : KEYEVENTF_KEYUP;

            if (vkCode == VK_CONTROL || vkCode == VK_SHIFT ||
                vkCode == VK_MENU || vkCode == VK_UP || vkCode == VK_DOWN ||
                vkCode == VK_LEFT || vkCode == VK_RIGHT) {
                flags |= KEYEVENTF_EXTENDEDKEY;
            }

            keybd_event(static_cast<BYTE>(vkCode), static_cast<BYTE>(scanCode), flags, 0);
            return;
        }

        UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
        LPARAM lParam = (scanCode << 16) | 1;

        if (!down) {
            lParam |= (1 << 30);
            lParam |= (1 << 31);
        }

        if (vkCode == VK_CONTROL || vkCode == VK_SHIFT ||
            vkCode == VK_MENU || vkCode == VK_UP || vkCode == VK_DOWN ||
            vkCode == VK_LEFT || vkCode == VK_RIGHT ||
            (vkCode >= VK_F1 && vkCode <= VK_F24)) {
            lParam |= (1 << 24);
        }

        if (down) {
            PostMessage(targetWindow, WM_KEYDOWN, vkCode, lParam);
        } else {
            PostMessage(targetWindow, WM_KEYUP, vkCode, lParam);
        }

        Sleep(1);
    }

    void SimulateMouseButton(int button, bool down) {
        DWORD flags = (button == 1 ?
            (down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP) :
            (down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP));
        mouse_event(flags, 0, 0, 0, 0);
    }

    void SimulateMouseMove(int dx, int dy) {
        // Use SendInput for relative movement and prevent Windows from coalescing moves.
        if (dx == 0 && dy == 0) return;

        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | 0x2000; // 0x2000 == MOUSEEVENTF_MOVE_NOCOALESCE
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