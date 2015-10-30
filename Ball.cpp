#include "Ball.h"
#include <gdiplus.h>

std::list<Ball*> Ball::Balls;

HBRUSH Ball::ChaosBrush[NumChaosBrush];
COLORREF Ball::ChaosColor[NumChaosBrush];
HBITMAP Ball::ChaosImage[NumChaosBrush];
HBITMAP Ball::BallImage;
HBITMAP Ball::ArrowImage;
HBITMAP Ball::ArrowMask;

extern GameState CurState;

VOID CALLBACK BallConnectProc(int x, int y, LPARAM lpData);
struct Connection{ HDC hdc; int srcx; int srcy; int dstx; int dsty; COLORREF src; COLORREF dst; };

BallManager::BallManager(HWND hWnd)
{
	Ball::BallImage = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3));
	Ball::ArrowImage = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP4));
	Ball::ArrowMask = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP5));
	HDC hdc = GetDC(hWnd);
	for (int i = 0; i < NumChaosBrush; ++i)
	{
		Ball::ChaosColor[i] = RGB(abs(RandInt()) % 256, abs(RandInt()) % 256, abs(RandInt()) % 256);
		Ball::ChaosBrush[i] = CreateSolidBrush(Ball::ChaosColor[i]);
		Ball::ChaosImage[i] = ColorizeBitmap(hdc, Ball::BallImage, Ball::ChaosColor[i], RGB(255, 0, 255), CurState.Bubble);
	}
	ReleaseDC(hWnd, hdc);
}
BallManager::~BallManager()
{
	DeleteObject(Ball::BallImage);
	DeleteObject(Ball::ArrowImage);
	for (int i = 0; i < NumChaosBrush; ++i)
	{
		DeleteObject(Ball::ChaosBrush[i]);
		DeleteObject(Ball::ChaosImage[i]);
	}
}

Ball::tag_BallInfo::tag_BallInfo(Vec2D Pos, Vec2D Vel) :Pos{ Pos }, Vel{ Vel }
{
	Top = true;
	Mobile = false;
	Enabled = true;
	myColor = RGB(abs(RandInt()) % 256, abs(RandInt()) % 256, abs(RandInt()) % 256);
	
	if (CurState.ChaosBall)
		Chaos = !(RandInt() % NumChaosBrush);
	else Chaos = false;
	
	Arrow = false; //default setting, as when coming down from ceiling.
}
Ball::tag_BallInfo::tag_BallInfo(tag_BallInfo& src)
{
	memcpy(this, &src, sizeof(tag_BallInfo)); //simple POD
}
Ball::tag_BallInfo::tag_BallInfo()
{
	;//don't do anything.
}

Ball::Ball(Vec2D Pos, Vec2D Vel) :BallInfo{ Pos, Vel }
{
	BallInfo.Arrow = false;
	Initialize();
}
Ball::Ball(Ball&src) : BallInfo{ src.BallInfo } //needed for restoration
{
	Initialize();
}
Ball::Ball(tag_BallInfo& Info) : BallInfo{ Info }
{
	Initialize();
}
Ball::Ball(Vec2D Pos, bool Fire) : BallInfo{ Pos, Vec2D{} }
{
	BallInfo.Top = false;
	if (CurState.Arrow && !BallInfo.Chaos) //arrow enabled & not chaos ball
		BallInfo.Arrow = !(RandInt() % ARROWPROBABILTY);
	Initialize();
}
Ball::~Ball()
{
	if (!BallInfo.Arrow && !BallInfo.Chaos)
	{
		DeleteObject(ImageBit);
		DeleteObject(myBrush);
	}
}

void Ball::Initialize()
{
	std::fill(Neighbors, Neighbors + 6, nullptr);
	std::fill(NeighborColor, NeighborColor+6, (BallInfo.Chaos==true)?true:false);
	BallInfo.Durability = ARROWDURABILITY;
	if (!BallInfo.Arrow)
	{
		//normal ball -- GET IMAGEBIT (NOT REALLY)
#ifdef LongCut
		if (!BallInfo.Chaos) //non-chaos
		{
			myBrush = CreateSolidBrush(BallInfo.myColor); //not chaos
		}
		RECT R = { 0, 0, 2 * Rad, 2 * Rad };
		HDC hdc = GetDC(hMainWnd);
		ImageBit = CreateCompatibleBitmap(hdc, Rad * 2, Rad * 2); //contains transparent image of Ball
		if (ImageBit == NULL)
		{
			int ReportError = 5;
			ReportError -= 5;
		}
		HDC ImageDC = CreateCompatibleDC(hdc); //private copy of ball
		HBITMAP ImageOldBit = (HBITMAP)SelectObject(ImageDC, ImageBit);

		HDC BallDC = CreateCompatibleDC(hdc);
		HBITMAP BallOldBit = (HBITMAP)SelectObject(BallDC, BallImage); //temporary

		FillRect(ImageDC, &R, (HBRUSH)GetStockObject(WHITE_BRUSH));
		TransparentBlt(ImageDC, 0, 0, 2 * Rad, 2 * Rad, BallDC, 0, 0, Rad * 2, Rad * 2, RGB(255, 0, 255));

		SelectObject(BallDC, BallOldBit);
		SelectObject(ImageDC, ImageOldBit);

		DeleteDC(BallDC);
		DeleteDC(ImageDC);
		ReleaseDC(hMainWnd, hdc);
		//ImageBit Remains.
		*/
#else
		//if normal ball, just apply filter here!
		if (!BallInfo.Chaos) //non-chaos
		{
			HDC hdc = GetDC(hMainWnd);
			myBrush = CreateSolidBrush(BallInfo.myColor); //not chaos
			ImageBit = ColorizeBitmap(hdc, Ball::BallImage, BallInfo.myColor, RGB(255, 0, 255), CurState.Bubble);
			ReleaseDC(hMainWnd, hdc);
		}
		//else already initialized(Chaos)
#endif
	}
}
void Ball::ReColor()
{
	HDC hdc = GetDC(hMainWnd);
	for (int i = 0; i < NumChaosBrush; ++i)
	{
		DeleteObject(ChaosImage[i]);
		ChaosImage[i] = ColorizeBitmap(hdc, Ball::BallImage, ChaosColor[i], RGB(255, 0, 255), CurState.Bubble);
	}

	for (auto b : Balls)
	{
		if (!b->BallInfo.Chaos)
			DeleteObject(b->ImageBit);

		b->ImageBit = ColorizeBitmap(hdc, Ball::BallImage, b->BallInfo.myColor, RGB(255, 0, 255), CurState.Bubble);
	}
	ReleaseDC(hMainWnd, hdc);
}
void Ball::PrivateReColor()
{
	HDC hdc = GetDC(hMainWnd);

	if (!BallInfo.Chaos && ImageBit)
	{
		DeleteObject(ImageBit);
		ImageBit = ColorizeBitmap(hdc, Ball::BallImage, BallInfo.myColor, RGB(255, 0, 255), CurState.Bubble);
	}
	
	ReleaseDC(hMainWnd, hdc);
}
void Ball::Print(HDC hdc)
{
	//HBRUSH OldBrush = (HBRUSH)SelectObject(hdc, myBrush);
	if (BallInfo.Chaos)
	{
		int index = abs(RandInt()) % NumChaosBrush;
		myBrush = ChaosBrush[index];
		BallInfo.myColor = ChaosColor[index];
		ImageBit = ChaosImage[index];
	}
	else if (BallInfo.Arrow) //special
	{
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, ArrowImage);

		double theta = BallInfo.Vel.Angle();
		Vec2D LU = BallInfo.Pos + (Vec2D{ -Rad, -Rad }).Rotate(theta);
		Vec2D RU = BallInfo.Pos + (Vec2D{ Rad, -Rad }).Rotate(theta);
		Vec2D LD = BallInfo.Pos + (Vec2D{ -Rad, Rad}).Rotate(theta);

		POINT RotP[] = { { LU.x, LU.y }, { RU.x, RU.y }, { LD.x, LD.y } };

		//BitBlt(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, Rad * 2, Rad * 2, MemDC, 0, 0, SRCCOPY);
		PlgBlt(hdc, RotP, MemDC, 0, 0, Rad*2, Rad*2,ArrowMask, 0, 0);

		SelectObject(MemDC, OldBit);
		DeleteDC(MemDC);
		return;
	}


	if (!BallInfo.Enabled)
	{
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP MemBit = CreateCompatibleBitmap(hdc, 2 * Rad, 2 * Rad);
		HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, MemBit);
		HBRUSH OldBrush = (HBRUSH)SelectObject(MemDC, myBrush);

		BLENDFUNCTION BF = { AC_SRC_OVER, 0, 120, 0};		

		RECT R = { 0, 0, Rad * 2, Rad * 2 };
		FillRect(MemDC, &R, (HBRUSH)GetStockObject(WHITE_BRUSH));
		Ellipse(MemDC, 0,0, Rad*2,Rad*2);
		//BitBlt(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, Rad * 2, Rad * 2, MemDC, 0, 0, SRCCOPY);
		AlphaBlend(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, 2 * Rad, 2 * Rad,
			MemDC, 0, 0, 2 * Rad, 2 * Rad, BF);
		
		SelectObject(MemDC, OldBit);
		SelectObject(MemDC, OldBrush);

		DeleteObject(MemBit);
		DeleteDC(MemDC);
	}
	else //BallInfo.Enabled
	{
		//
		//SelectObject(hdc, myBrush);
		//Ellipse(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, BallInfo.Pos.x + Rad, BallInfo.Pos.y + Rad);
//#define LongCut
#ifdef LongCut
		RECT R = { 0, 0, Rad * 2, Rad * 2 };

		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP MemBit = CreateCompatibleBitmap(hdc, Rad * 2, Rad * 2);
		HBITMAP MemOldBit = (HBITMAP)SelectObject(MemDC, MemBit);
		FillRect(MemDC, &R, (HBRUSH)GetStockObject(WHITE_BRUSH));
		HBRUSH MemOldBrush = (HBRUSH)SelectObject(MemDC, myBrush);
		Ellipse(MemDC, 0,0,2*Rad,2*Rad); //Get Color on Ball

		//add image
		
		HDC ImageDC = CreateCompatibleDC(hdc); //copy of ball
		HBITMAP ImageOldBit = (HBITMAP)SelectObject(ImageDC, ImageBit);

		BLENDFUNCTION BF = { AC_SRC_OVER, 0, 120, 0 };
		AlphaBlend(MemDC, 0, 0, Rad * 2, Rad * 2, ImageDC, 0, 0, Rad * 2, Rad * 2, BF);


		TransparentBlt(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, 2 * Rad, 2 * Rad, MemDC, 0, 0, Rad * 2, Rad * 2, RGB(255, 255, 255));

		SelectObject(ImageDC, ImageOldBit);
		SelectObject(MemDC, MemOldBit);
		SelectObject(MemDC, MemOldBrush);

		DeleteObject(MemBit);
		DeleteDC(ImageDC);
		DeleteDC(MemDC);
#else

		//HDC MemDC = CreateCompatibleDC(hdc);
		//HBITMAP ColorBit = ColorizeBitmap(hdc, Ball::BallImage, BallInfo.myColor, RGB(255, 0, 255));
		//HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, ColorBit);
		//TransparentBlt(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, 2 * Rad, 2 * Rad, MemDC, 0, 0, Rad * 2, Rad * 2, RGB(255, 0, 255));
		//SelectObject(MemDC, OldBit);
		//DeleteObject(MemDC);
		//DeleteObject(ColorBit);

		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, ImageBit);
		static BLENDFUNCTION BF = { AC_SRC_OVER, 0, 255,AC_SRC_ALPHA };
		if (CurState.Bubble)
		{
			AlphaBlend(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, 2 * Rad, 2 * Rad, MemDC, 0, 0, 2 * Rad, 2 * Rad, BF);
		}
		else
		{
			TransparentBlt(hdc, BallInfo.Pos.x - Rad, BallInfo.Pos.y - Rad, 2 * Rad, 2 * Rad, MemDC, 0, 0, Rad * 2, Rad * 2, RGB(255, 0, 255));
		}

		SelectObject(MemDC, OldBit);
		DeleteObject(MemDC);

#endif

		if (CurState.ShowConnection)
		{
			
			for (int i = 0; i < 6; ++i)
			{
				if (Neighbors[i] && NeighborColor[i])
				{
					auto n = Neighbors[i];

					Vec2D V_1 = BallInfo.Pos + (n->BallInfo.Pos - BallInfo.Pos).Rotate(PI / 3).Truncate(Rad * 2/3);
					Vec2D V_2 = BallInfo.Pos + (n->BallInfo.Pos - BallInfo.Pos).Rotate(5*PI/3).Truncate(Rad*2/3);

#define Weaken(a) (((a)+210)/2)
					TRIVERTEX V[3] =
					{
						{ V_1.x, V_1.y, Weaken(GetRValue(BallInfo.myColor)) * 256, Weaken(GetGValue(BallInfo.myColor)) * 256, Weaken(GetBValue(BallInfo.myColor)) * 256, 0xff00 },
						{ V_2.x, V_2.y, Weaken(GetRValue(BallInfo.myColor)) * 256, Weaken(GetGValue(BallInfo.myColor)) * 256, Weaken(GetBValue(BallInfo.myColor)) * 256, 0xff00 },
						{ n->BallInfo.Pos.x, n->BallInfo.Pos.y, Weaken(GetRValue(n->BallInfo.myColor)) * 256, Weaken(GetGValue(n->BallInfo.myColor)) * 256, Weaken(GetBValue(n->BallInfo.myColor)) * 256, 0xff00 }
					};
					GRADIENT_TRIANGLE GT = { 0, 1, 2 };

					GdiGradientFill(hdc, V, 3, &GT, 1, GRADIENT_FILL_TRIANGLE);
				}
			}
#ifdef DynamicColorCheck
			for (auto n : Neighbors)
			{
				if (n)
				{
					if (SimilarColor(BallInfo.myColor, n->BallInfo.myColor)) //huge calculation overhead...how to resolve...?
					{
						//1. a rectangle
						//Vec2D MidPoint = (BallInfo.Pos + n->BallInfo.Pos) / 2;
						//Rectangle(hdc, MidPoint.x - 4, MidPoint.y - 4, MidPoint.x + 4, MidPoint.y + 4);

						//2. a connecting line
						//MoveToEx(hdc, BallInfo.Pos.x, BallInfo.Pos.y, NULL);
						//LineTo(hdc, n->BallInfo.Pos.x, n->BallInfo.Pos.y);

						//3. a special functional line
						//Connection C = { hdc, BallInfo.Pos.x, BallInfo.Pos.y, n->BallInfo.Pos.x, n->BallInfo.Pos.y, BallInfo.myColor, n->BallInfo.myColor };
						//LineDDA(BallInfo.Pos.x, BallInfo.Pos.y, n->BallInfo.Pos.x, n->BallInfo.Pos.y, BallConnectProc, (LPARAM)&C);
						//4. GDI Gradient

						Vec2D V_1 = BallInfo.Pos + (n->BallInfo.Pos - BallInfo.Pos).Rotate(PI / 2).Truncate(Rad / 3);
						Vec2D V_2 = BallInfo.Pos + (n->BallInfo.Pos - BallInfo.Pos).Rotate(3 * PI / 2).Truncate(Rad / 3);

						TRIVERTEX V[3] =
						{
							{ V_1.x, V_1.y, GetRValue(BallInfo.myColor) * 256, GetGValue(BallInfo.myColor) * 256, GetBValue(BallInfo.myColor) * 256, 0xff00 },
							{ V_2.x, V_2.y, GetRValue(BallInfo.myColor) * 256, GetGValue(BallInfo.myColor) * 256, GetBValue(BallInfo.myColor) * 256, 0xff00 },
							{ n->BallInfo.Pos.x, n->BallInfo.Pos.y, GetRValue(n->BallInfo.myColor) * 256, GetGValue(n->BallInfo.myColor) * 256, GetBValue(n->BallInfo.myColor) * 256, 0xff00 }
						};
						GRADIENT_TRIANGLE GT = { 0, 1, 2 };

						GdiGradientFill(hdc, V, 3, &GT, 1, GRADIENT_FILL_TRIANGLE);
					}
				}
			}
#endif
		}
	}

	
	
	//TCHAR index[] = TEXT("0");
	//for (auto i = Balls.begin(); i != Balls.end(); ++i, ++index[0])
	//	if (this == *i) break;

	//for (auto n : Neighbors) if (n) ++index[0];
	//SetTextAlign(hdc, TA_CENTER|TA_BOTTOM);
	//TextOut(hdc, BallInfo.Pos.x, BallInfo.Pos.y, index, 1);
	
	//if (!BallInfo.Enabled)
	//	SelectObject(hdc, GetStockObject(LTGRAY_BRUSH));

	//SelectObject(hdc, OldBrush);
}
void Ball::Print(HDC hdc, int x, int y)
{
	Vec2D Pos = BallInfo.Pos;
	BallInfo.Pos.x = x, BallInfo.Pos.y = y;
	Print(hdc);
	BallInfo.Pos.x = Pos.x;
	BallInfo.Pos.y = Pos.y;
}
void Ball::Move()
{
	if (BallInfo.Mobile)
	{	
		//BallInfo.Pos += (BallInfo.Vel += (Acc *= 0.99)) *= 0.95;
		BallInfo.Pos += BallInfo.Vel;
		if (BallInfo.Pos.x < 0) //bounce against wall
		{
			BallInfo.Pos.x += BallInfo.Vel.x = abs(BallInfo.Vel.x);
		}
		else if (BallInfo.Pos.x >BoardWidth)
		{
			BallInfo.Pos.x += BallInfo.Vel.x = -abs(BallInfo.Vel.x);
		}

	}
}

bool Ball::Touch(Ball* other)
{
	return (BallInfo.Pos - other->BallInfo.Pos).Magnitude() <= 2 * Rad + 1;
}



void Ball::Attach(Ball* other) //always, I am "Moving", and the other is still.
{
	if (BallInfo.Arrow)
	{
		if (--BallInfo.Durability < 1)
			Detach();
		other->Detach();
		return;
	}

	BallInfo.Mobile = false;
	int R_T = RegionType((BallInfo.Pos - other->BallInfo.Pos).Angle());
	switch (R_T)
	{
	case -1:
		//significant error
		break;
	case 0:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ Rad * 2, 0 };
		break;
	case 5:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ Rad, -Rad*sqrt(3) };
		break;
	case 4:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ -Rad, -Rad*sqrt(3) };
		break;
	case 3:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ -Rad * 2, 0 };
		break;
	case 2:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ -Rad, Rad*sqrt(3) };
		break;
	case 1:
		BallInfo.Pos = other->BallInfo.Pos + Vec2D{ Rad, Rad*sqrt(3) };
		break;
	}
	if (Neighbors[R_T]) //already neighbors
		return;

	other->Neighbors[(R_T-3)<0?R_T+3:R_T-3] = this; //each other
	Neighbors[R_T] = other;

	if (SimilarColor(BallInfo.myColor, other->BallInfo.myColor))
	{
		NeighborColor[R_T] = true;
		other->NeighborColor[(R_T - 3)<0 ? R_T + 3 : R_T - 3] = true;
	}

	GlobalConnect();
}

void Ball::Attach() //to ceiling
{
  	if (BallInfo.Arrow)
	{
		Detach();
		return;
	}

	BallInfo.Mobile = false;
	BallInfo.Top = true;

	//BallInfo.Vel.x = BallInfo.Vel.y = Acc.x = Acc.y = 0;

	BallInfo.Pos.y = Rad;
	switch (CurState.Align)
	{
	case GameState::tag_Align::Left:
		BallInfo.Pos.x = floor(BallInfo.Pos.x / (2 * Rad)) * 2 * Rad + Rad;
		break;
	case GameState::tag_Align::Right:
		BallInfo.Pos.x = floor((BallInfo.Pos.x-Rad) / (2 * Rad)) * 2 * Rad + 2*Rad;
		break;
	}
	GlobalConnect();
}

void Ball::Detach() //from all else.
{
	for (int i = 0; i < 6; ++i)
	{
		if (Neighbors[i])
		{
			Neighbors[i]->Detach((i - 3) < 0 ? i + 3 : i - 3);
			Neighbors[i] = nullptr;
		}
	}

	//for (auto n : Neighbors)
	//{ 
	//	if (n)
	//		n->Detach(this); // for them.
	//}

	BallInfo.Mobile = true;
	BallInfo.Enabled = false;
	BallInfo.Vel.y = 30; //falling
	BallInfo.Vel.x = RandInt() % 10;
}
void Ball::Detach(Ball* neighbor)
{
	for (int i = 0; i < 6; ++i)
	{
		if (Neighbors[i] == neighbor)
		{
			Neighbors[i] = nullptr;
			NeighborColor[i] = false; //no matter what.
		}
	}

	if (!BallInfo.Top && Neighbors[1] == nullptr && Neighbors[2] == nullptr) //nothing on top.
		Detach();
}
void Ball::Detach(int index)
{
	Neighbors[index] = nullptr;
	NeighborColor[index] = false;

	if (!BallInfo.Top && Neighbors[1] == nullptr && Neighbors[2] == nullptr) //nothing on top.
		Detach();
}
bool Ball::OutOfBound()
{
	return BallInfo.Pos.y > BoardHeight;
}

bool Ball::Ceiling()
{
	return BallInfo.Pos.y < Rad;
}

bool Ball::isMobile(){ return BallInfo.Mobile; }
bool Ball::isEnabled(){ return BallInfo.Enabled; }
void Ball::Propagate(std::list<Ball*>& FallList, COLORREF Color)
{
	if (std::find(FallList.begin(), FallList.end(), this) == FallList.end()) //no find
	{
		if (BallInfo.Chaos || SimilarColor(BallInfo.myColor, Color)) //if I am chaos, propagate no matter what.
		{
			FallList.push_back(this);
			for (auto n : Neighbors)
			{
				if (n) //not nullptr
				{
					n->Propagate(FallList, BallInfo.Chaos?n->BallInfo.myColor : BallInfo.myColor); //propagating, now with my own color
				}
			}
		}
	}

}

void Ball::CheckFall()
{
	std::list<Ball*> FallList;
	Propagate(FallList, BallInfo.myColor);

	if (FallList.size() > 2)
	{
		FallList.sort([](Ball* a, Ball* b){
			return (a->BallInfo.Pos.y < b->BallInfo.Pos.y);
		});
		//Sort FallList to avoid double-visit.
		//i.e. start falling from top!

		//Scoring
		CurState.Score += FallList.size()*FallList.size() * 10;
		InvalidateRect(hPop, NULL, TRUE);
		//
		for (auto& f : FallList)
		{
			f->Detach();
		}
	}
}

void Ball::MoveDown()
{
	for (auto &B : Balls) //preexisting balls
	{
		B->BallInfo.Top = false;
		if (!B->isMobile())
		{
			B->BallInfo.Pos.y += sqrt(3) * Rad;
		}
	}


	if (CurState.Align == GameState::tag_Align::Left)
	{
		CurState.Align = GameState::tag_Align::Right;
		for (int i = Rad*2; i <= BoardWidth - Rad; i += Rad * 2)
		{
			Ball* newBall = new Ball(Vec2D(i, Rad));
			Balls.push_back(newBall);
			newBall->GlobalConnect();
		}
	}
	else if (CurState.Align == GameState::tag_Align::Right)
	{
		CurState.Align = GameState::tag_Align::Left;
		for (int i = Rad; i <= BoardWidth-Rad; i += Rad * 2)
		{
			Ball* newBall = new Ball(Vec2D(i, Rad));
			Balls.push_back(newBall);
			newBall->GlobalConnect();
		}
	}

}

void Ball::Fire(Vec2D Dir)
{
	BallInfo.Enabled = true;
	BallInfo.Mobile = true;
	BallInfo.Vel = Dir;
}
void Ball::GlobalConnect()
{
	for (auto& B : Balls)
	{
		if (B != this && Touch(B) && !B->isMobile())
		{
			Attach(B);
		}
	}
}
void Ball::Bounce(Ball* a, Ball* b)
{
	Vec2D Va = a->BallInfo.Vel;
	Vec2D Vb = b->BallInfo.Vel;
	a->BallInfo.Vel = Va - (a->BallInfo.Pos - b->BallInfo.Pos)*((Va - Vb)*(a->BallInfo.Pos - b->BallInfo.Pos) / (a->BallInfo.Pos - b->BallInfo.Pos).MagnitudeSQ());
	b->BallInfo.Vel = Vb - (b->BallInfo.Pos - a->BallInfo.Pos)*((Vb - Va)*(b->BallInfo.Pos - a->BallInfo.Pos) / (b->BallInfo.Pos - a->BallInfo.Pos).MagnitudeSQ());
}
void Ball::ResetColorWeb()
{
	for (int i = 0; i < 6; ++i)
	{
		if (Neighbors[i] && SimilarColor(BallInfo.myColor, Neighbors[i]->BallInfo.myColor))
		{
			NeighborColor[i] = true;
		}
		else if (BallInfo.Chaos)
			NeighborColor[i] = true;
		else
			NeighborColor[i] = false;
	}
}
//different gradation coloring scheme

VOID CALLBACK BallConnectProc(int x, int y, LPARAM lpData)
{

	Connection* c = (Connection*)lpData;
	//SetPixel(c->hdc, x, y, (c->dst*Ratio + c->src*(1-Ratio)) / 2);
	
	double Ratio = (double)(c->srcx - x) / (c->srcx - c->dstx);
	COLORREF NewColor = RGB(GetRValue(c->dst)*Ratio + GetRValue(c->src)*(1 - Ratio),
		GetGValue(c->dst)*Ratio + GetGValue(c->src)*(1 - Ratio),
		GetBValue(c->dst)*Ratio + GetBValue(c->src)*(1 - Ratio));
	//COLORREF NewColor = c->src*(1 - Ratio) + c->dst*Ratio;
	HBRUSH myBrush = CreateSolidBrush(NewColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(c->hdc, myBrush);
	SelectObject(c->hdc, GetStockObject(NULL_PEN));
	Ellipse(c->hdc, x - 2, y - 2, x + 2, y + 2);

	SelectObject(c->hdc, oldBrush);
	DeleteObject(myBrush);
}