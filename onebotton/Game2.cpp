#include "Game2.h"

void Game2::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	srand((unsigned int)time(nullptr));
	m_State = State::WAIT;
}

void Game2::Uninit()
{
}

void Game2::Update()
{
	switch (m_State)
	{
	case State::WAIT:
		// スペース押す → 開始準備
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			m_State = State::READY;
			m_StartTime = GetTickCount() + (rand() % 2000 + 1000); // 1～3秒後に開始
		}
		break;

	case State::READY:
		// 開始時間を超えたら「押せ！」状態へ
		if (GetTickCount() >= m_StartTime)
		{
			m_StartTime = GetTickCount();
			m_State = State::RESULT;
		}
		break;

	case State::RESULT:
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			m_ReactionTime = GetTickCount() - m_StartTime;
			m_State = State::WAIT;
		}
		break;
	}
}

void Game2::Draw()
{
	HDC hdc = GetDC(m_hWnd);
	RECT rc;
	GetClientRect(m_hWnd, &rc);

	FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));

	char text[256];

	switch (m_State)
	{
	case State::WAIT:
		wsprintfA(text, "SPACEでスタート");
		break;

	case State::READY:
		wsprintfA(text, "......");
		break;

	case State::RESULT:
		wsprintfA(text, "PUSH!!");
		break;
	}

	DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 結果表示
	if (m_ReactionTime > 0)
	{
		char result[256];
		wsprintfA(result, "反応速度: %d ms", m_ReactionTime);
		rc.top += 100;
		DrawTextA(hdc, result, -1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE);
	}

	ReleaseDC(m_hWnd, hdc);
}

