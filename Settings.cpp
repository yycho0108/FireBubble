#include "Header.h"
#include "resource.h"
#include "GameState.h"
#include "Ball.h"
#include "ScoreReport.h"
#include "Self.h"

#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")

extern GameState CurState;
extern HINSTANCE g_hInst;

struct ToolTipHelper
{
	TOOLINFO TIF;
	ToolTipHelper()
	{
		INITCOMMONCONTROLSEX ICCE;
		ICCE.dwSize = sizeof(INITCOMMONCONTROLSEX);
		ICCE.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&ICCE);

		
		TIF = {};
		TIF.cbSize = TTTOOLINFOW_V1_SIZE;
		TIF.hinst = GetModuleHandle(NULL);
	}
	HWND RegisterTooltip(HWND hDlg)
	{
		HWND hToolTip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, TOOLTIPS_CLASS,
			NULL, WS_CHILD | WS_POPUP | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, TIF.hinst, 0);
SetWindowPos(hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
SendMessage(hToolTip, TTM_ACTIVATE, TRUE, 0);
return hToolTip;
	}
	void AddTip(HWND hToolTip, HWND SubCtl, LPTSTR Text)
	{
		TIF.uId = (UINT_PTR)SubCtl;
		TIF.hwnd = GetParent(hToolTip);
		TIF.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
		TIF.lpszText = Text;

		SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&TIF);
	}
	void AddTip(HWND hToolTip, UINT_PTR SubCtlID, LPTSTR Text)
	{
		AddTip(hToolTip, GetDlgItem(GetParent(hToolTip), SubCtlID), Text);
	}
} TTH;
void ResetSettings(HWND hDlg)
{
	SetWindowText(GetDlgItem(hDlg, IDC_EDIT3), TEXT(""));

	SendDlgItemMessage(hDlg, IDC_CHECK1, BM_SETCHECK, TRUE,0);
	SendDlgItemMessage(hDlg, IDC_CHECK2, BM_SETCHECK, TRUE,0);
	SendDlgItemMessage(hDlg, IDC_CHECK3, BM_SETCHECK, TRUE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK4, BM_SETCHECK, TRUE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK5, BM_SETCHECK, TRUE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK6, BM_SETCHECK, TRUE, 0);

	SendDlgItemMessage(hDlg, IDC_CHECK7, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK8, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK9, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK10, BM_SETCHECK, FALSE, 0);

	SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, 1);
	SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_SETPOS, TRUE, 1024);
}
BOOL CALLBACK OptionsProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
	{

		SetWindowText(GetDlgItem(hDlg, IDC_EDIT3), CurState.Name);
		SendDlgItemMessage(hDlg, IDC_CHECK1, BM_SETCHECK, CurState.ToggleGuide ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK2, BM_SETCHECK, CurState.EnableDiscard ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK3, BM_SETCHECK, CurState.ChaosBall ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK4, BM_SETCHECK, CurState.Bounce ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK5, BM_SETCHECK, CurState.ShowConnection ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK6, BM_SETCHECK, CurState.Arrow ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK7, BM_SETCHECK, CurState.Music ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK8, BM_SETCHECK, CurState.BkPic ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK9, BM_SETCHECK, CurState.PopSound ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK10, BM_SETCHECK, CurState.Bubble ? BST_CHECKED : BST_UNCHECKED, 0);
		
		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGE, TRUE, MAKELPARAM(0, 40));
		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, CurState.Difficulty);

		SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_SETRANGEMIN, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_SETRANGEMAX, 0, 43768);
		SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_SETPOS, TRUE, CurState.Tolerance);


		HWND hToolTip = TTH.RegisterTooltip(hDlg);

		//SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
		//SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
		//TTH.AddTip(hToolTip, IDC_SLIDER1, LPSTR_TEXTCALLBACK);
		//TTH.AddTip(hToolTip, IDC_SLIDER2, LPSTR_TEXTCALLBACK);

		//FORMAT : TTH.AddTip(hToolTip, 00, TEXT(""));

		TTH.AddTip(hToolTip, IDC_COLSENSE, TEXT("Leeway of <Similar Color>"));

		TTH.AddTip(hToolTip, IDC_LINEDESC, TEXT("Speed of Ball Regeneration from Top"));

		TTH.AddTip(hToolTip, IDC_CHECK7, TEXT("David Oistrakh, Legende by Wieniawski"));
		TTH.AddTip(hToolTip, IDC_CHECK8, TEXT("Simple Bubble Background"));
		TTH.AddTip(hToolTip, IDC_CHECK3, TEXT("Chaos Ball Binds to All Its Neighbors"));
		TTH.AddTip(hToolTip, IDC_CHECK6, TEXT("Arrow Pierces Through Balls"));
		TTH.AddTip(hToolTip, IDC_CHECK1, TEXT("Guide Hints to Ball Path"));
		TTH.AddTip(hToolTip, IDC_CHECK2, TEXT("Throw Away Current Ball"));
		TTH.AddTip(hToolTip, IDC_CHECK4, TEXT("Balls Will Bounce Against Each Other in Air"));
		TTH.AddTip(hToolTip, IDC_CHECK5, TEXT("Display Connection between Similar-Colored Balls"));
		TTH.AddTip(hToolTip, IDC_CHECK9, TEXT("Simple Pop Sound"));
		TTH.AddTip(hToolTip, IDC_CHECK10, TEXT("TransParent Bubbles"));

		TTH.AddTip(hToolTip, IDC_BUTTON1, TEXT("Help"));
		TTH.AddTip(hToolTip, IDC_BUTTON2, TEXT("Rollback Settings to Default"));
		TTH.AddTip(hToolTip, IDC_EDIT3, TEXT("Enter Player Name"));
		return TRUE;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			CurState.ToggleGuide = SendDlgItemMessage(hDlg, IDC_CHECK1, BM_GETCHECK, 0, 0);
			CurState.EnableDiscard = SendDlgItemMessage(hDlg, IDC_CHECK2, BM_GETCHECK, 0, 0);
			CurState.ChaosBall = SendDlgItemMessage(hDlg, IDC_CHECK3, BM_GETCHECK, 0, 0);
			CurState.Bounce = SendDlgItemMessage(hDlg, IDC_CHECK4, BM_GETCHECK, 0, 0);
			CurState.ShowConnection = SendDlgItemMessage(hDlg, IDC_CHECK5, BM_GETCHECK, 0, 0);
			CurState.Arrow = SendDlgItemMessage(hDlg, IDC_CHECK6, BM_GETCHECK, 0, 0);
			CurState.Music = SendDlgItemMessage(hDlg, IDC_CHECK7, BM_GETCHECK, 0, 0);
			CurState.BkPic = SendDlgItemMessage(hDlg, IDC_CHECK8, BM_GETCHECK, 0, 0);
			CurState.PopSound = SendDlgItemMessage(hDlg, IDC_CHECK9, BM_GETCHECK, 0, 0);
			CurState.Bubble = SendDlgItemMessage(hDlg, IDC_CHECK10, BM_GETCHECK, 0, 0);

			CurState.Difficulty = SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
			CurState.Tolerance = SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_GETPOS, 0, 0);
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT3), CurState.Name, 256);

			SimilarColor.SetTolerance(CurState.Tolerance);

			Ball::ReColor();
			if (Me)
				Me->ReColorBall();

			for (auto b : Ball::Balls)
				b->ResetColorWeb();


			InvalidateRect(hPop, NULL, TRUE);
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDC_BUTTON1:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, HelpProc);
			return TRUE;
		case IDC_BUTTON2:
		{
			ResetSettings(hDlg);
			return TRUE;
		}
		}
		return FALSE;
	}
	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case TTN_GETDISPINFO:
		{
			LPNMTTDISPINFO LInfo = (LPNMTTDISPINFO)lParam;
			int Pos = SendMessage((HWND)LInfo->hdr.idFrom, TBM_GETPOS, 0, 0);
			if (LInfo->hdr.idFrom == (UINT_PTR)GetDlgItem(hDlg, IDC_SLIDER1)) //Difficulty
			{
				tsprintf_s(LInfo->szText, TEXT("Current Interval : Every [%.2f] Second(s)"), (20000 - 500 * Pos) / 1000.0);
			}
			else if (LInfo->hdr.idFrom == (UINT_PTR)GetDlgItem(hDlg, IDC_SLIDER2))
			{
				tsprintf_s(LInfo->szText, TEXT("Current Threshold: 0<[%d]<43768"), Pos);
			}
			return TRUE;
		}
		}
		return FALSE;
	}

	}
	return FALSE;
}

BOOL CALLBACK HelpProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
	}
	return FALSE;
}