#include "Header.h"
#include "GameState.h"

#define FBKey TEXT("Software\\YOYO\\FireBubbleKey")//Ranking-registry?
#define RECBUFSIZE 256

struct Record
{
	HKEY Key;
	HKEY StateKey;
	HKEY ScoreKey;
	HKEY NameKey;
	Record();
	~Record();

	int SetRecord(unsigned long long score, TCHAR* Name);
	void GetRecord(TCHAR* RecBuf, size_t MaxLen);

	void SetScore(int index, unsigned long long score);
	unsigned long long GetScore(int index);

	bool SetName(int index, TCHAR* Name);
	bool GetName(int index, TCHAR* Name, int MaxLen);

	bool SetState(GameState State);
	bool GetState(GameState* State);

	void DisplayRanking();
	void Reset();
};

extern Record CurRec;