#pragma once
#include <Windows.h>
#include "Renderer.h"
#include<string>

class Game2
{
public:
    enum class State
    {
        WAIT,
        READY,
        RESULT,   // Œv‘ª’†
        SHOW,     // •\Ž¦—p
        FOUL,
    };


    void Init(HWND hWnd);
    void Uninit();
    void Update();
    void Draw();

private:
    HWND m_hWnd = nullptr;

    State m_State = State::WAIT;

    DWORD m_StartTime = 0;
    DWORD m_ReactionTime = 0;

    HDC m_hDC;
    HBITMAP m_hBmp;
    HDC m_memDC;
    DWORD m_FoulTime;

};
