#include "Header.h"
#include "ScoreReport.h"
#include "GameState.h"

LRESULT CALLBACK ScoreReportProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

LPCTSTR ScTitle = TEXT("Report");
ScoreWindow ScoreReport;
extern GameState CurState;

ScoreWindow::ScoreWindow()
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(g_hInst, IDI_APPLICATION);
	wc.hInstance = g_hInst;
	wc.lpfnWndProc = ScoreReportProc;
	wc.lpszClassName = ScTitle;
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClass(&wc);
}
HWND ScoreWindow::CreatePopupWindow()
{
	HWND PopupWindow = CreateWindow(ScTitle,ScTitle, WS_POPUP | WS_VISIBLE | WS_CAPTION,
		10, 10,200,160, hMainWnd, (HMENU)NULL, g_hInst, NULL);
	return PopupWindow;
}

LRESULT CALLBACK ScoreReportProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT R;
	static TCHAR TextBuf[128];
	switch (iMsg)
	{
	case WM_CREATE:
	{
		GetClientRect(hWnd, &R);
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		tsprintf_s(TextBuf, TEXT("Your Name : %s\nYour Score : %llu\nYour Ball :"), CurState.Name,CurState.Score);
		DrawText(hdc, TextBuf, lstrlen(TextBuf), &R, DT_WORDBREAK | DT_CENTER);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_SIZE:
	{
		GetClientRect(hWnd, &R);
		return 0;
	}
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}