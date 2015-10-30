#include "Self.h"

Self::Self(Vec2D Pos) :Pos{ Pos }
{
	Turn = Dir::NONE;
	theta = PI / 2;
	Image = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
	Mask = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));
	Aim = CreatePen(PS_DOT, 1, RGB(200, 0, 200));
	MyBall = new Ball(Pos, true);
	RotSpeed = 0;
}
Self::~Self()
{
	DeleteObject(Image);
	DeleteObject(Mask);
	DeleteObject(Aim);

	delete MyBall;
	MyBall = nullptr;
}
void Self::Fire()
{
	if (CurState.PopSound)
		mciSendString(TEXT("play PopSound from 0"), NULL, NULL, NULL);
	MyBall->Fire(Vec2D{ 10 * cos(theta), -10 * sin(theta) });
	Ball::Balls.push_back(MyBall);
	MyBall = new Ball(Me->Pos, true);
}

void Self::Discard()
{
	MyBall->Fire(Vec2D{ -10 * cos(theta), 10 * sin(theta) }); //opp.dir
	Ball::Balls.push_back(MyBall);
	MyBall = new Ball(Me->Pos, true);
}

void Self::Rotate()
{
	/*
	switch (Turn)
	{
	case CCLOCK:
	theta += AtoR(3);
	if (theta > PI) theta -= 2 * PI;
	break;
	case CLOCK:
	theta -= AtoR(3);
	if (theta < -PI) theta += 2 * PI;
	break;
	}

	*/
	switch (Turn)
	{
	case CCLOCK:
		RotSpeed += 0.5;
		break;
	case CLOCK:
		RotSpeed -= 0.5;
		break;
	}
	theta += AtoR(RotSpeed);

	if (theta > PI) theta -= 2 * PI;
	else if (theta < -PI) theta += 2 * PI;
}

void Self::SetRotate(Dir dir)
{
	Turn = (Dir)(Turn | dir);
}

void Self::StopRotate(Dir dir)
{
	RotSpeed = 0;
	Turn = (Dir)(Turn & (~dir));
}

void Self::Print(HDC hdc)
{
	HDC MemDC = CreateCompatibleDC(hdc);
	HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, Image);
	HPEN OldPen = (HPEN)SelectObject(hdc, Aim);

	Vec2D LU = Pos + (Vec2D{ -24, -24 }).Rotate(-theta);
	Vec2D RU = Pos + (Vec2D{ 24, -24 }).Rotate(-theta);
	Vec2D LD = Pos + (Vec2D{ -24, 24 }).Rotate(-theta);
	POINT RotP[] = { { LU.x, LU.y }, { RU.x, RU.y }, { LD.x, LD.y } };

	//aim

	if (CurState.ToggleGuide)
	{
		MoveToEx(hdc, Pos.x, Pos.y, NULL);
		LineTo(hdc, Pos.x + 400 * cos(theta), Pos.y - 400 * sin(theta));

	}
	MyBall->Print(hdc);

	//
	HDC PopDC = GetDC(hPop);
	RECT R;
	GetClientRect(hPop, &R);
	RECT R2 = { R.right / 2 - Rad, R.bottom - 2 * Rad, R.right / 2 + Rad, R.bottom };
	FillRect(PopDC, &R2, (HBRUSH)GetStockObject(WHITE_BRUSH));
	MyBall->Print(PopDC, R.right / 2, R.bottom - Rad);
	ReleaseDC(hPop, PopDC);
	//

	PlgBlt(hdc, RotP, MemDC, 0, 0, 48, 48, Mask, 0, 0);

	SelectObject(MemDC, OldBit);
	SelectObject(hdc, OldPen);
	DeleteDC(MemDC);
}

void Self::ReColorBall()
{
	if (MyBall)
		MyBall->PrivateReColor();
}