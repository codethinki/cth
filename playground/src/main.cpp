#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

// Configuration
const int TOTAL_SAMPLES = 100;
const UINT TEST_KEY = 0x58; // F24 is invisible and won't mess up your windows

// Global state
std::chrono::high_resolution_clock::time_point start_time;
std::vector<long long> latencies;
HHOOK hHook = NULL;

void TriggerNextInput() {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = TEST_KEY;

    start_time = std::chrono::high_resolution_clock::now();
    if (SendInput(1, &input, sizeof(INPUT)) == 0) {
        std::cerr << "SendInput failed! Error: " << GetLastError() << std::endl;
        PostQuitMessage(0);
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        auto const pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        
        if (pKeyBoard->vkCode == TEST_KEY) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            
            latencies.push_back(duration);

            // Clean up the key (Key Up)
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = TEST_KEY;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));

            if (latencies.size() < TOTAL_SAMPLES) {
                TriggerNextInput(); // Chain reaction
            } else {
                PostQuitMessage(0); // Finished!
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
    std::cout << "Starting benchmark (100 samples)..." << std::endl;

    // 1. Set the hook
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!hHook) {
        std::cerr << "Failed to install hook!" << std::endl;
        return 1;
    }

    // 2. Kick off the first event
    TriggerNextInput();

    // 3. The Message Loop (The "Heartbeat" of Windows Input)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 4. Cleanup and Results
    UnhookWindowsHookEx(hHook);

    if (latencies.empty()) {
        std::cout << "No data collected. Are you running as Admin? Is an Antivirus blocking hooks?" << std::endl;
    } else {
        long long min_v = latencies[0], max_v = latencies[0], sum = 0;
        for (auto l : latencies) {
            if (l < min_v) min_v = l;
            if (l > max_v) max_v = l;
            sum += l;
        }

        std::cout << "\n--- Latency Results (Microseconds) ---" << std::endl;
        std::cout << "Average: " << sum / latencies.size() << " us" << std::endl;
        std::cout << "Minimum: " << min_v << " us" << std::endl;
        std::cout << "Maximum: " << max_v << " us" << std::endl;
        std::cout << "Total processed: " << latencies.size() << std::endl;
    }

    return 0;
}