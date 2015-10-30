
#include "Record.h"
#include "resource.h"

BOOL CALLBACK RankProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
//HIGH SCORES
Record CurRec;
Record::Record()
{
	DWORD dwDisp;
	RegCreateKeyEx(HKEY_CURRENT_USER, FBKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &Key, &dwDisp);
	RegCreateKeyEx(Key, TEXT("STATE"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &StateKey, &dwDisp);
	RegCreateKeyEx(Key, TEXT("SCORE"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &ScoreKey, &dwDisp);
	RegCreateKeyEx(Key, TEXT("NAME"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &NameKey, &dwDisp);
	//NAME = Tentative.
	//cuz I have to create a dialog box...ish.
}
Record::~Record()
{
	RegCloseKey(Key);
	RegCloseKey(StateKey);
	RegCloseKey(ScoreKey);
	RegCloseKey(NameKey);
}

void Record::SetScore(int index, unsigned long long score)
{
	TCHAR Rank[3] = TEXT("00");
	tsprintf_s(Rank, TEXT("%02d"), index);
	RegSetValueEx(ScoreKey, Rank, NULL, REG_QWORD, (LPBYTE)&score, sizeof(score));
}

int Record::SetRecord(unsigned long long Score, TCHAR* Name)
{
	unsigned long long PrevScore;
	TCHAR PrevName[RECBUFSIZE];
	int RankIndex = INT_MAX;

	for (int i = 1; i <= 10; ++i)
	{
		PrevScore = GetScore(i);
		if (Score > PrevScore) ///~Propagate this way.
		{
			if(i<RankIndex)
				RankIndex = i;
			GetName(i, PrevName, RECBUFSIZE);

			SetScore(i, Score);
			SetName(i, Name);

			Name = PrevName;
			Score = PrevScore;
		}
	}
	return RankIndex;
}
void Record::GetRecord(TCHAR* RecBuf,size_t MaxLen)
{
	TCHAR* RecPtr = RecBuf;
	TCHAR NameBuf[RECBUFSIZE];
	for (int i = 1; i <= 10; ++i)
	{
		GetName(i, NameBuf, RECBUFSIZE);
		tsprintf_s(RecPtr, MaxLen, TEXT("%d. %-10s : %010llu\n"), i,NameBuf, GetScore(i));
		MaxLen -= lstrlen(RecPtr);
		RecPtr = RecBuf + lstrlen(RecBuf);
	}
}
unsigned long long Record::GetScore(int index)
{
	if (index < 1 || index>10)
		return 0;

	TCHAR Rank[3] = TEXT("00");
	tsprintf_s(Rank, TEXT("%02d"), index);

	QWORD Score;
	DWORD cBytes;
	if (RegQueryValueEx(ScoreKey, Rank, NULL, NULL, (LPBYTE)&Score,&cBytes) != ERROR_SUCCESS)
		return 0;
	return Score;
}

bool Record::SetName(int index, TCHAR* Name)
{
	TCHAR Rank[3] = TEXT("00");
	tsprintf_s(Rank, TEXT("%02d"), index);
	RegSetValueEx(NameKey, Rank, NULL, REG_SZ, (LPBYTE)Name, (lstrlen(Name) + 1)*sizeof(TCHAR));
	return true;
}
bool Record::GetName(int index, TCHAR* Name, int MaxLen)
{
	TCHAR Rank[3] = TEXT("00");
	tsprintf_s(Rank, TEXT("%02d"), index);

	DWORD NameLen;
	if (RegQueryValueEx(NameKey, Rank, NULL, NULL, NULL, &NameLen) != ERROR_SUCCESS)
		lstrcpy(Name, TEXT("ANON"));
	if (NameLen > MaxLen)
		return false;
	if (RegQueryValueEx(NameKey, Rank, NULL, NULL, (LPBYTE)Name, &NameLen) != ERROR_SUCCESS)
		lstrcpy(Name, TEXT("ANON"));
	return true;
}

bool Record::SetState(GameState State) //just the settings
{
	RegSetValueEx(StateKey, NULL, NULL,REG_BINARY, (LPBYTE)&State, sizeof(State));
	//RegSetValueEx(StateKey, TEXT("PLAYERNAME"), NULL, REG_BINARY, (LPBYTE)State.Name, sizeof(TCHAR)*(lstrlen(State.Name)+1));
	return true;
}
bool Record::GetState(GameState* State)
{
	DWORD cBytes = sizeof(*State);
	RegQueryValueEx(StateKey, NULL, NULL, NULL, (LPBYTE)State, &cBytes);
	//RegQueryValueEx(StateKey, TEXT("PLAYERNAME"), NULL,NULL, (LPBYTE)State->Name, &cBytes);
	return true;
}

void Record::DisplayRanking()
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hMainWnd, RankProc);
	//CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hMainWnd, RankProc, Rank);
}

void Record::Reset()
{
	RegDeleteTree(ScoreKey, NULL);
	RegDeleteTree(NameKey, NULL);
}
BOOL CALLBACK RankProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
	{
		TCHAR NameBuf[128];
		TCHAR ScoreBuf[32];
		for (int i = 0; i < 10; ++i)
		{
			CurRec.GetName(1+i, NameBuf, 128);
			tsprintf_s(ScoreBuf, TEXT("%llu"), CurRec.GetScore(1+i));
			SetWindowText(GetDlgItem(hDlg, IDC_NAME1 + i), NameBuf);
			SetWindowText(GetDlgItem(hDlg, IDC_SCORE1 + i), ScoreBuf);
		}
		return TRUE;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDC_BUTTON1:
		{
			CurRec.Reset();
			for (int i = 0; i < 10; ++i)
			{
				SetWindowText(GetDlgItem(hDlg, IDC_NAME1 + i), TEXT("ANON"));
				SetWindowText(GetDlgItem(hDlg, IDC_SCORE1 + i), TEXT("0"));
			}
		}
		}
		return FALSE;
	}
	}
	return FALSE;
}