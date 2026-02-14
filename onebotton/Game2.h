#pragma once
#include <Windows.h>

class Game2
{
public:
	void Init(HWND hWnd);
	void Uninit();
	void Update();
	void Draw();

private:
	enum class State
	{
		WAIT,
		READY,
		RESULT
	};

	State m_State = State::WAIT;

	DWORD m_StartTime = 0;
	DWORD m_ReactionTime = 0;

	HWND m_hWnd = nullptr;
};

