#include "Game2.h"
#include <time.h>

void Game2::Init(HWND hWnd)
{
    m_hWnd = hWnd;
    m_hDC = GetDC(hWnd);

    srand((unsigned int)time(nullptr));

    m_State = State::WAIT;
    m_StartTime = 0;
    m_ReactionTime = 0;

    RECT rc;
    GetClientRect(m_hWnd, &rc);

    m_memDC = CreateCompatibleDC(m_hDC);
    m_hBmp = CreateCompatibleBitmap(m_hDC, rc.right, rc.bottom);
    SelectObject(m_memDC, m_hBmp);

}

void Game2::Uninit()
{
    DeleteObject(m_hBmp);
    DeleteDC(m_memDC);
    ReleaseDC(m_hWnd, m_hDC);
}

void Game2::Update()
{
    static bool prev = false;
    bool now = (GetAsyncKeyState(VK_SPACE) & 0x8000);

    switch (m_State)
    {  
    case State::WAIT:
        if (now && !prev)
        {
            m_State = State::READY;
            m_StartTime = GetTickCount() + (rand() % 2000 + 1000);
        }
        break;

    case State::READY:
        // ★ フライング判定
        if (now && !prev)
        {
            m_State = State::FOUL;
            m_FoulTime = GetTickCount();
            break;
        }

        if (GetTickCount() >= m_StartTime)
        {
            m_StartTime = GetTickCount();   // 計測開始
            m_State = State::RESULT;
        }
        break;
        break;

    case State::RESULT:
        if (now && !prev)
        {
            m_ReactionTime = GetTickCount() - m_StartTime;
            m_State = State::SHOW;   // ★ 表示用ステートへ
        }
        break;

    case State::SHOW:
        if (now && !prev)
        {
            m_State = State::WAIT;   // 次のゲームへ
        }
        break;

    case State::FOUL:
        if (GetTickCount() - m_FoulTime > 1500)  // 1.5秒表示
        {
            m_State = State::WAIT;
        }
        break;
    }

    

    prev = now;
}



void Game2::Draw()
{
    RECT rc;
    GetClientRect(m_hWnd, &rc);

    // バックバッファをクリア（白）
    HBRUSH brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    FillRect(m_memDC, &rc, brush);

    SetBkMode(m_memDC, TRANSPARENT);
    SetTextColor(m_memDC, RGB(0, 0, 0));

    HFONT hFont = CreateFontA(
        48, 0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    HFONT oldFont = (HFONT)SelectObject(m_memDC, hFont);

    char text[256];

    switch (m_State)
    {
    case State::WAIT:
        sprintf_s(text, "PRESS SPACE");
        break;

    case State::READY:
        sprintf_s(text, "WAIT...");
        break;

    case State::RESULT:
        sprintf_s(text, "PLESS!!!");
        break;

    case State::SHOW:
        sprintf_s(text, "REACTION TIME : %d ms\nPRESS SPACE", m_ReactionTime);
        break;

    case State::FOUL:
        sprintf_s(text, "FALSE START!\nTOO EARLY!");
        break;

    }

    DrawTextA(
        m_memDC,
        text,
        -1,
        &rc,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_SINGLELINE
    );

    SelectObject(m_memDC, oldFont);
    DeleteObject(hFont);

    // 画面に転送
    BitBlt(m_hDC, 0, 0, rc.right, rc.bottom, m_memDC, 0, 0, SRCCOPY);
}


