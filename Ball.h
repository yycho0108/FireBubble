#pragma once

#include "Header.h"
#include "Vec2D.h"
#include "resource.h"
#include "GameState.h"
#include <list>

#define NumChaosBrush (20)
#define ARROWDURABILITY (10)
#define ARROWPROBABILTY (30)

#pragma comment (lib, "Msimg32.lib")

class Ball
{
	friend class BallManager;
	
	friend bool Load();
	friend bool Save();

private:
	static HBRUSH ChaosBrush[NumChaosBrush];
	static COLORREF ChaosColor[NumChaosBrush];
	static HBITMAP ChaosImage[NumChaosBrush];
	static HBITMAP BallImage;
	static HBITMAP ArrowImage;
	static HBITMAP ArrowMask;
	//KEY INFO
	struct tag_BallInfo
	{
		Vec2D Pos;
		Vec2D Vel;
		COLORREF myColor;
		bool Mobile;
		bool Enabled;
		bool Top;
		bool Chaos; //Chaos Ball
		bool Arrow; //Arrow Ball
		BYTE Durability; //for an Arrow Ball
		tag_BallInfo(Vec2D Pos, Vec2D Vel);
		tag_BallInfo(tag_BallInfo& src);//copy
		tag_BallInfo();
	} BallInfo;

	//DISPOSABLE INFO
	Ball* Neighbors[6];
	bool NeighborColor[6];
	HBITMAP ImageBit;
	HBRUSH myBrush;

public:
	static std::list<Ball*> Balls;
	Ball(Vec2D Pos = {}, Vec2D Vel = {});
	Ball(Ball& src);
	Ball(tag_BallInfo& Info);
	Ball(Vec2D Pos, bool Fire);
	~Ball();
	virtual void Print(HDC hdc);
	void Print(HDC hdc, int x, int y);
	virtual void Move();
	void Initialize();
	bool Touch(Ball* other);
	bool OutOfBound();
	bool Ceiling();
	bool isMobile();
	bool isEnabled();
	void Attach(Ball* other);
	void Attach();
	void Propagate(std::list<Ball*>& BallList, COLORREF Color);
	void CheckFall();
	void Detach();
	void Detach(Ball* neighbor);
	void Detach(int index);
	void Fire(Vec2D Dir);
	void GlobalConnect();
	void ResetColorWeb();
	static void MoveDown();
	static void Bounce(Ball* a, Ball* b);
	static void ReColor();
	void PrivateReColor();
	//static void setHWND(HWND hWnd);
};

class BallManager
{
public:
	BallManager(HWND hWnd);
	~BallManager();
};