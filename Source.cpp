
#include "Header.h"
#include "Ball.h"
#include "GameState.h"
#include "Record.h"
#include "ScoreReport.h"
#include "Self.h"

#include "resource.h"
#include <Windows.h>
#include <Commctrl.h>
#include <list>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Winmm.lib")

ATOM RegisterCustomClass(HINSTANCE& hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

VOID CALLBACK FrameProc(HWND hWnd, UINT iMsg, UINT_PTR CallerID, DWORD dwTime);
VOID CALLBACK DifficultyProc(HWND hWnd, UINT iMsg, UINT_PTR CallerID, DWORD dwTime);

void EndGame(bool Final);
void NewGame(bool Initial);

void Pause();
void Resume();

bool Save();
bool Load();


LPCTSTR Title = L"FireBubble";
HINSTANCE g_hInst;
HWND hMainWnd;
HWND hPop;

HBITMAP MemBit;
GameState CurState;
HBRUSH BkBrush;
HBITMAP BkBit;

Self* Me;


ATOM RegisterCustomClass(HINSTANCE& hInstance)
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = Title;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	return RegisterClass(&wc);
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInstance;
	RegisterCustomClass(hInstance);
	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, BoardWidth,BoardHeight, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

BallManager* Initializer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT R; //Client Rect
	switch (iMsg)
	{
	case WM_CREATE:
	{
		InitCommonControls();
		hMainWnd = hWnd;
		CurRec.GetState(&CurState);
		Initializer = new BallManager(hWnd);
		mciSendString(TEXT("open BkMusic.mp3 type mpegvideo alias BkMusic"), NULL, NULL,hWnd);
		mciSendString(TEXT("open Pop.mp3 type mpegvideo alias PopSound"), NULL, NULL, hWnd);

		//mciSendString(TEXT("seek PopSound to end"), NULL, NULL, hWnd);
		//mciSendString(TEXT("seek BkMusic to end"), NULL, NULL, hWnd);

		mciSendString(TEXT("play PopSound"), NULL, NULL, hWnd);
		mciSendString(TEXT("play BkMusic"), NULL, NULL, hWnd);

		if (DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, OptionsProc) == IDCANCEL)
			return -1;
		if (!CurState.Music)
			Pause();
		hPop = ScoreReport.CreatePopupWindow();

		NewGame(true);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEWGAME:
			if (CurState.OnGoing)
			{
				if (MessageBox(hWnd, TEXT("A game is ongoing. Start a new session?"), TEXT("WARNING"), MB_YESNO) == IDYES)
				{
					EndGame(true);
					NewGame(true);
				}
			}
			else
			{
				NewGame(true);
			}
			break;
		case ID_FILE_ENDGAME:
			EndGame(true);
			break;
		case ID_FILE_PAUSE:
		{
			if (CurState.Pause)
				Resume();
			else
				Pause();
			break;
		}
		case ID_FILE_SAVE:
		{
			if (!Save())
				MessageBox(hWnd, TEXT("Couldn't Save State"), TEXT("ERROR"), MB_OK);
			break;
		}
		case ID_FILE_LOAD:
		{
			if (!Load())
				MessageBox(hWnd, TEXT("Couldn't Load State"), TEXT("ERROR"), MB_OK);
			break;
		}
		case ID_FILE_RANKING:
		{
			CurRec.DisplayRanking();
			break;
		}
		case ID_FILE_SETTINGS:
			Pause();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, OptionsProc);
			Resume();
			break;
		case ID_FILE_HELP:
			Pause();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, HelpProc);
			Resume();
			break;
		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_LEFT:
		case 'A':
			if (!CurState.Pause)
				Me->SetRotate(Self::Dir::CCLOCK);
			break;
		case 'D':
		case VK_RIGHT:
			if (!CurState.Pause)
				Me->SetRotate(Self::Dir::CLOCK);
			break;
		case 'Z':
		case VK_SPACE:
			if (!CurState.Pause)
 				Me->Fire();
			break;
		case VK_TAB:
			if (CurState.EnableDiscard && (!CurState.Pause))
				Me->Discard();
			break;
		case VK_RETURN: //PAUSE
			if (CurState.Pause)
			{
				Resume();
			}
			else
			{
				Pause();
			}
			break;
		}
		break;
	}
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_LEFT:
		case 'A':
			Me->StopRotate(Self::Dir::CCLOCK);
			break;
		case 'D':
		case VK_RIGHT:
			Me->StopRotate(Self::Dir::CLOCK);
			break;
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, MemBit);

		BitBlt(hdc, 0, 0, R.right, R.bottom, MemDC, 0, 0, SRCCOPY);

		SelectObject(MemDC, OldBit);
		DeleteDC(MemDC);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZE:
	{
		//R.left = R.top = 0;
		R.right = LOWORD(lParam);
		R.bottom = HIWORD(lParam);
		break;
	}
	case MM_MCINOTIFY:
	{
		if (wParam == MCI_NOTIFY_SUCCESSFUL)
		{
			mciSendString(TEXT("play BkMusic from 0 notify"), NULL, NULL, hMainWnd);
		}
		break;
	}
	case WM_ACTIVATEAPP:
	{
		if (wParam == TRUE)
		{
			ShowWindow(hPop, SW_SHOW);
			Resume();
		}
		else
		{
			ShowWindow(hPop, SW_HIDE);
			Pause();
		}
		break;
	}
	case WM_DESTROY:
		EndGame(true);
		delete Initializer;
		CurRec.SetState(CurState);
		mciSendString(TEXT("close BkMusic"), NULL, NULL, NULL);
		mciSendString(TEXT("close PopSound"), NULL, NULL, NULL);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}



//GAME PROCESS
VOID CALLBACK FrameProc(HWND hWnd, UINT iMsg, UINT_PTR CallerID, DWORD dwTime)
{
	HDC hdc = GetDC(hWnd);
	HDC MemDC = CreateCompatibleDC(hdc);
	HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, MemBit);

	RECT R;
	GetClientRect(hWnd, &R);
	FillRect(MemDC, &R, BkBrush);

	SetMapMode(MemDC, MM_ANISOTROPIC);
	SetWindowExtEx(MemDC, BoardWidth, BoardHeight, NULL);
	SetViewportExtEx(MemDC, R.right, R.bottom, NULL);

	//FILL BK with Image
	if (CurState.BkPic)
	{
		HDC BKDC = CreateCompatibleDC(hdc);
		HBITMAP OldBKBit = (HBITMAP)SelectObject(BKDC, BkBit);
		BitBlt(MemDC, 0, 0, BoardWidth, BoardHeight, BKDC, 0, 0, SRCCOPY);
		SelectObject(BKDC, OldBKBit);
		DeleteDC(BKDC);
	}

	//

	Me->Rotate();
	Me->Print(MemDC);

	for (auto i = Ball::Balls.begin(); i != Ball::Balls.end();)
	{
		Ball* B = *i++;
		B->Move();

		if (B->OutOfBound())
		{
			if (!B->isMobile())
			{
				return EndGame(false);
			}
			else
			{
  				Ball::Balls.remove(B);
				delete B;
				continue;
			}
		}

		for (auto j = Ball::Balls.begin(); j != Ball::Balls.end(); ++j)
		{
			Ball* b = *j;
			if (B != b && B->Touch(b) && B->isEnabled() && b->isEnabled() && B->isMobile())
			{
				if (b->isMobile()) //both mobile
				{
					if (CurState.Bounce)
						Ball::Bounce(B, b);
				}
				else
				{
					B->Attach(b);
					B->CheckFall();
					break;
				}
			}
		}

		if (B->isMobile() && B->Ceiling())
		{
			B->Attach();
			B->CheckFall();
		}
		B->Print(MemDC);
	}

	SelectObject(MemDC, OldBit);
	DeleteDC(MemDC);
	ReleaseDC(hWnd, hdc);


	InvalidateRect(hWnd, NULL, FALSE);


	return;
}

VOID CALLBACK DifficultyProc(HWND hWnd, UINT iMsg, UINT_PTR CallerID, DWORD dwTime)
{
	Ball::MoveDown();
}


//GAME FUNCTIONS
void EndGame(bool Final)
{
	Pause();
	
	DeleteObject(MemBit);
	DeleteObject(BkBrush);
	DeleteObject(BkBit);

	CurState.OnGoing = false;
	delete Me;
	Me = nullptr;

	for (auto B : Ball::Balls)
		delete B;

	Ball::Balls.clear();

	if (!Final)
	{
		int Rank = CurRec.SetRecord(CurState.Score, CurState.Name);
		if (Rank != INT_MAX)
		{
			TCHAR RankIndex[128];
			tsprintf_s(RankIndex, TEXT("Your Rank is : %d"), Rank);
			MessageBox(hMainWnd, RankIndex, TEXT("Congratulations!"), MB_OK);
			CurRec.DisplayRanking();
		}
		if (MessageBox(hMainWnd, TEXT("CONTINUE?"), TEXT("GAME OVER"), MB_YESNO) == IDYES)
		{
			return NewGame(true);
		}
		else
			DestroyWindow(hMainWnd);
	}

}

void NewGame(bool Initial)
{
	SetWindowText(hMainWnd, TEXT("[FireBubble]"));
	BkBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
	HDC hdc = GetDC(hMainWnd);
	MemBit = CreateCompatibleBitmap(hdc, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));
	BkBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(0, 0, 0));
	ReleaseDC(hMainWnd, hdc);

	if (Initial) //not loading from savedata
	{
		CurState.Score = 0;
		CurState.Align = GameState::tag_Align::Left;
		for (int i = 0; i < 4; ++i)
		{
			Ball::MoveDown(); //4ÁÙ »ý¼º
		}
	}

	CurState.OnGoing = true;

	Me = new Self(Vec2D(BoardWidth / 2, BoardHeight - 48));

	if (CurState.Music)
		mciSendString(TEXT("play BkMusic from 0 notify"), NULL, NULL, hMainWnd);

	InvalidateRect(hPop, NULL, TRUE);
	Resume();
}


void Pause()
{
	mciSendString(TEXT("pause BkMusic"), NULL, NULL, NULL);
	CurState.Pause = true;
	KillTimer(hMainWnd, 0); //FrameProc
	KillTimer(hMainWnd, 1);

	MENUITEMINFO PauseInfo = {};
	PauseInfo.cbSize = sizeof(MENUITEMINFO);
	PauseInfo.fMask = MIIM_STRING;
	PauseInfo.dwTypeData = CurState.Pause ? L"Resume" : L"Pause";
	SetMenuItemInfo(GetSubMenu(GetMenu(hMainWnd), 0), 2, TRUE, &PauseInfo);

}

void Resume()
{
	if (CurState.Music)
		mciSendString(TEXT("resume BkMusic"), NULL, NULL, NULL);
	CurState.Pause = false;
	SetTimer(hMainWnd, 0, 20, FrameProc);
	SetTimer(hMainWnd, 1, 20000 - 500 * CurState.Difficulty, DifficultyProc);

	MENUITEMINFO PauseInfo = {};
	PauseInfo.cbSize = sizeof(MENUITEMINFO);
	PauseInfo.fMask = MIIM_STRING;
	PauseInfo.dwTypeData = CurState.Pause ? L"Resume" : L"Pause";
	SetMenuItemInfo(GetSubMenu(GetMenu(hMainWnd), 0), 2, TRUE, &PauseInfo);

}

bool Save()
{
	TCHAR FilePath[MAX_PATH];
	int SaveNum = 0;
	DWORD cBytes;
	HANDLE hFile;
	do
	{
		tsprintf_s(FilePath, TEXT("save%02d.bin"), ++SaveNum);
		hFile = CreateFile(FilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (SaveNum > 99) //either error, or too many save files. either way, stop.
		{
			MessageBox(hMainWnd, TEXT("TOO MANY SAVE FILES"), TEXT("ERROR"), MB_OK);
			return false;
		}
	} while (hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS);
	DWORD StateInfoSize = sizeof(CurState);
	DWORD BallInfoSize = sizeof(Ball::tag_BallInfo);

	WriteFile(hFile, &StateInfoSize, sizeof(StateInfoSize), &cBytes, NULL);
	WriteFile(hFile, &CurState, StateInfoSize, &cBytes, NULL);
	WriteFile(hFile, &BallInfoSize, sizeof(BallInfoSize), &cBytes, NULL);
	for (auto b : Ball::Balls)
	{
		WriteFile(hFile, &b->BallInfo, BallInfoSize, &cBytes, NULL);
	}
	MessageBox(hMainWnd, FilePath, TEXT("Save Successful"), MB_OK);
	CloseHandle(hFile);
	return true;
}

bool Load()
{
	TCHAR FilePath[MAX_PATH] = {};
	DWORD cBytes;
	HANDLE hFile;
	EndGame(true);
	if (!FileName(hMainWnd, FilePath, TEXT("Binary File(*.bin*)\0*.bin*\0\0"), MAX_PATH))
		return false;//wrong path
	//get FilePath
	hFile = CreateFile(FilePath, GENERIC_READ,FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD StateInfoSize;
	DWORD BallInfoSize;
	Ball::tag_BallInfo InfoBuf{};

	ReadFile(hFile, &StateInfoSize, sizeof(StateInfoSize), &cBytes, NULL);
	ReadFile(hFile, &CurState, StateInfoSize, &cBytes, NULL);
	ReadFile(hFile, &BallInfoSize, sizeof(BallInfoSize), &cBytes, NULL);
	while (ReadFile(hFile, &InfoBuf,BallInfoSize, &cBytes, NULL) && cBytes)
	{
		Ball::Balls.push_back(new Ball(InfoBuf));
	}
	CloseHandle(hFile);
	for (auto b : Ball::Balls)
	{
		b->GlobalConnect();
	}
	InvalidateRect(hPop, NULL, TRUE);
	NewGame(false);
	return true;
}