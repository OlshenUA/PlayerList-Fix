#pragma once
#include <Windows.h>
#include <WinUser.h>
#include <strsafe.h>

class KInput
{
public:
    enum InputState
    {
        IDLE,
        PRESSED,
        STAY,
        RELEASED
    };

private:
    static int m_keys[256];
    static KInput::InputState m_keysState[256];
    static double m_horizontal;
    static double m_vertical;
    static double m_normalizationTime;
public:
    KInput() {}
    ~KInput() {}
    static KInput& Instance() {
        static KInput instance;
        return instance;
    }
    static void Update(double elapsedTime_);

    static double GetAxis(const char* name) {
        if (_stricmp(name, "Horizontal") == 0)
            return m_horizontal;
        else if (_stricmp(name, "Vertical") == 0)
            return m_vertical;
        return 0.0;
    }
    static bool GetKeyDown(int vkey) {
        return m_keys[vkey] & 0x01;
    }
    static KInput::InputState GetKeyState(int vkey) {
        return m_keysState[vkey];
    }
};

#define Input   KInput::Instance()
