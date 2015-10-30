#include "Header.h"

tag_SimilarColor SimilarColor;
std::function<int()> RandInt = std::bind(std::uniform_int_distribution < int > {INT_MIN, INT_MAX}, std::default_random_engine{ unsigned int(time(0)) });
//int RandInt()
//{
//	static std::function<int()> R_I = std::bind(std::uniform_int_distribution < int > {INT_MIN, INT_MAX}, std::default_random_engine{ unsigned int(time(0)) });
//	return R_I();
//}

inline double AtoR(int Angle)
{
	return Angle*PI / 180;
};
inline double RtoA(double Radian)
{
	return Radian * 180 / PI;
};
int RegionType(double theta)
{
	int Angle = RtoA(theta);
	if (-30 <= Angle && Angle < 30)
		return 0;
	if (30 <= Angle && Angle < 90)
		return 1;
	if (90 <= Angle && Angle < 150)
		return 2;
	if (150 <= Angle || Angle < -150)
		return 3;
	if (-150 <= Angle && Angle < -90)
		return 4;
	if (-90 <= Angle && Angle < -30)
		return 5;
	//error
	return -1;
}
int RTToAngle(int RegionType)
{
	switch (RegionType)
	{
	case 0:
		return 0;
	case 1:
		return 60;
	case 2:
		return 120;
	case 3:
		return 180;
	case 4:
		return -120;
	case 5:
		return -60;
	}
}


double tag_SimilarColor::Sq(double src)
{
	return src*src;
}
double tag_SimilarColor::Convert_1(double Col)
{
	return (Col>0.04045) ?
		pow((Col + 0.055) / 1.055, 2.4)
		: Col / 12.92;
}
double tag_SimilarColor::Convert_2(double Col)
{
	return Col > 0.008856 ?
		pow(Col, 1.0 / 3)
		: 7.787*Col + 16.0 / 116;
}
CIE_Lab tag_SimilarColor::RGBtoLab(COLORREF Color)
{
	double R = Convert_1(GetRValue(Color) / 255.0) * 100;
	double G = Convert_1(GetGValue(Color) / 255.0) * 100;
	double B = Convert_1(GetBValue(Color) / 255.0) * 100;

	double X = R *(0.412453) + G*(0.357580) + B*(0.180423);
	double Y = R*(0.212671) + G*(0.715160) + B*(0.072169);
	double Z = R*(0.019334) + G*(0.119193) + B*(0.950227);

	X = Convert_2(X / 95.047);
	Y = Convert_2(Y / 100.000);
	Z = Convert_2(Z / 108.883);

	double L = 116 * X - 16;
	double a = 500 * (X - Y);
	double b = 200 * (Y - Z);
	return CIE_Lab{ L, a, b };
}

bool tag_SimilarColor::operator()(COLORREF a, COLORREF b)
{
	//UINT BDist = (GetBValue(a) - GetBValue(b))*(GetBValue(a) - GetBValue(b));
	//UINT GDist = (GetGValue(a) - GetGValue(b))*(GetGValue(a) - GetGValue(b));
	//UINT RDist = (GetRValue(a) - GetRValue(b))*(GetRValue(a) - GetRValue(b));

	//return BDist + GDist + RDist < 10000;

	CIE_Lab A = RGBtoLab(a);
	CIE_Lab B = RGBtoLab(b);

	double Diff = Sq(A.L - B.L) + Sq(A.a - B.a) + Sq(A.b - B.b); // max = 42768

	return Diff < Tolerance;
}
void tag_SimilarColor::SetTolerance(int T)
{
	Tolerance = T;
}

TCHAR* FileName(HWND hWnd, TCHAR* lpstrFile, LPCWSTR lpstrFilter, int MaxLen)
{
	TCHAR InitDir[MAX_PATH];
	OPENFILENAME OFN = {};
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWnd;
	OFN.lpstrFilter = lpstrFilter;
	OFN.lpstrFile = lpstrFile;
	OFN.nMaxFile = MaxLen;
	OFN.lpstrTitle = TEXT("PLEASE SELECT FILE");
	//OFN.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	GetWindowsDirectory(InitDir, MAX_PATH);
	OFN.lpstrInitialDir = InitDir;
	if (GetOpenFileName(&OFN))
		return lpstrFile;
	else
		return nullptr;
}

//COLOR FILTER
static BOOL CompleteBitmapInfo(HBITMAP hBmp, BITMAPINFO* pbmi)
{
	BITMAP bmo;

	if (GetObject(hBmp, sizeof(BITMAP), &bmo))
	{
		
		ZeroMemory(pbmi, sizeof(BITMAPINFO));
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = bmo.bmWidth;
		pbmi->bmiHeader.biHeight = -bmo.bmHeight;
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = 32; /* Request a 32bpp bitmap. */
		pbmi->bmiHeader.biCompression = BI_RGB;
		return TRUE;
	}

	return FALSE;
}
PDWORD GetBitmapPixels(HBITMAP hBmp, HDC hdc)
{
	BITMAPINFO   bmi;
	PDWORD       pixels = nullptr;

	if (CompleteBitmapInfo(hBmp, &bmi))
	{
		pixels = (PDWORD)malloc(-bmi.bmiHeader.biHeight * bmi.bmiHeader.biWidth * sizeof(DWORD));

		if (pixels)
		{
			if (GetDIBits(hdc, hBmp, 0, -bmi.bmiHeader.biHeight, pixels, &bmi, DIB_RGB_COLORS))
			{
				return pixels;
			}
		}
	}

	free(pixels);
	return nullptr;
}

BOOL SetBitmapPixels(HBITMAP hBmp, HDC hdc, PDWORD pixels)
{
	BITMAPINFO   bmi;

	if (CompleteBitmapInfo(hBmp, &bmi))
	{
		if (SetDIBits(hdc, hBmp, 0, -bmi.bmiHeader.biHeight, pixels, &bmi, DIB_RGB_COLORS))
		{
			return TRUE;
		}
	}
	return FALSE;
}

inline BYTE Avg(BYTE a, BYTE b) //or weight?
{
	return a / 2 + b / 2;
}
inline COLORREF ApplyFilter(COLORREF Src, COLORREF Filter)
{
	//in BGR ORDER
	return RGB(Avg(GetRValue(Src), GetBValue(Filter)),
		Avg(GetGValue(Src), GetGValue(Filter)),
		Avg(GetBValue(Src), GetRValue(Filter)));
}
inline COLORREF ApplyFilter(COLORREF Src, COLORREF Filter, double Alpha)
{
	//in BGR ORDER
	return RGB(Avg(GetRValue(Src), GetBValue(Filter))*Alpha,
		Avg(GetGValue(Src), GetGValue(Filter))*Alpha,
		Avg(GetBValue(Src), GetRValue(Filter))*Alpha) | (int(Alpha) << 24);
}

#define SQR(n) ((n)*(n))
HBITMAP ColorizeBitmap(HDC hdc, HBITMAP hBitmap, COLORREF Filter, COLORREF Mask, bool Bubble)
{
	PDWORD   pixels = nullptr;
	BITMAP   bmp;
	COLORREF col;

	pixels = GetBitmapPixels(hBitmap, hdc);
	if (!pixels)
		return nullptr;

	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	HBITMAP DstBit = CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);

	for (int i = 0; i < bmp.bmHeight; ++i)
	{
		for (int j = 0; j < bmp.bmWidth; ++j)
		{
			col = pixels[i*bmp.bmWidth + j] & 0x00ffffff;
			if (col == Mask)
			{
				pixels[i*bmp.bmWidth + j] = Mask;
			}
			else
			{
				if (Bubble)
				{
					int AlphaFactor = (SQR(i - bmp.bmHeight / 2) + SQR(j - bmp.bmWidth / 2)) * 0xff
						/ (bmp.bmHeight*bmp.bmWidth / 4);
					if (AlphaFactor>255) AlphaFactor = 255;
					pixels[i*bmp.bmWidth + j] = ApplyFilter(col, Filter, AlphaFactor / 256.0) | (AlphaFactor << 24);
				}
				else
				{
					pixels[i*bmp.bmWidth + j] = ApplyFilter(col, Filter);
				}
			
			}
		}
	}

	SetBitmapPixels(DstBit,hdc, pixels); //to different output
	free(pixels);

	return DstBit;
}