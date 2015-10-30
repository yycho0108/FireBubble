#pragma once

typedef unsigned long DWORD;

#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif

struct GameState
{
	//struct size
	//DWORD cbSize;
	//Setting
	int Difficulty;
	int Tolerance;
	bool EnableDiscard;
	bool ToggleGuide;
	bool ChaosBall;
	bool Arrow;
	bool Bounce;
	bool ShowConnection;
	//Current State
	long long unsigned int Score;
	enum tag_Align{ Left, Right } Align;
	bool Pause;
	bool OnGoing;
	//extension Parameter
	TCHAR Name[256];
	//extension State flags
	bool Music;
	bool BkPic;
	bool PopSound;
	bool Bubble;

	GameState();
	~GameState();
};