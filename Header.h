#pragma once

#define BoardWidth (500)
#define BoardHeight (800) //logical
#define Rad (20)
#define PI (3.1415926535897932384626433832795)

#include <random>
#include <functional>
#include <ctime>
#include <Windows.h>

#ifdef _UNICODE
typedef  std::wstring tstring;
#define tsprintf_s swprintf_s
#else
typedef std::string tstring
#define tsprintf_s sprintf_s
#endif
typedef unsigned long long QWORD;
/* Random */
//extern int RandInt();

extern std::function<int()> RandInt;

/* Angles */
extern int RegionType(double theta);
extern int RTToAngle(int RegionType);
extern inline double AtoR(int Angle);
extern inline double RtoA(double Radian); //used in Balls.cpp

/* Color Detection/Manipulation */
struct CIE_Lab
{
	double L;
	double a;
	double b;
};
struct tag_SimilarColor
{

private:
	int Tolerance;
	static inline double Sq(double src);
	static inline double Convert_1(double Col);
	static inline double Convert_2(double Col);
	CIE_Lab RGBtoLab(COLORREF Color);
public:
	tag_SimilarColor(){Tolerance = 1024;}
	bool operator()(COLORREF a, COLORREF b);
	void SetTolerance(int T);
};
extern HBITMAP ColorizeBitmap(HDC hdc, HBITMAP hBitmap, COLORREF Filter, COLORREF Mask, bool Bubble);

/* Settings */
extern BOOL CALLBACK OptionsProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK HelpProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

/*File I/O*/
extern TCHAR* FileName(HWND hWnd, TCHAR* lpstrFile, LPCWSTR lpstrFilter, int MaxLen);


extern HWND hMainWnd;
extern HWND hPop;

extern HINSTANCE g_hInst;

extern tag_SimilarColor SimilarColor;