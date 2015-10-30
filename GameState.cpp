#include "GameState.h"


GameState::GameState()
{
	Difficulty = 0;
	Tolerance = 1024;
	EnableDiscard = true;
	ToggleGuide = true;
	ChaosBall = true;
	Arrow = true;
	Bounce = true;
	ShowConnection = true;
	Score = 0;
	Pause = true;
	OnGoing = false;
	Align = tag_Align::Left;
	Music = true;
	BkPic = false;
	Bubble = false;
}


GameState::~GameState()
{
}
