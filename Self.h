#include "Header.h"
#include "Vec2D.h"
#include "resource.h"
#include "Ball.h"
#include "GameState.h"

extern GameState CurState;

struct Self
{
public:
	enum Dir{ NONE, CLOCK, CCLOCK, BOTH };
private:
	Vec2D Pos;
	Dir Turn;
	//no pos, no vel
	HBITMAP Image;
	HBITMAP Mask;
	HPEN Aim;
	double theta; // from -PI ~ PI
	Ball* MyBall;
	double RotSpeed;
public:
	Self(Vec2D Pos = {});
	~Self();
	void Fire();
	void Discard();
	void Rotate();
	void SetRotate(Dir dir);
	void StopRotate(Dir dir);
	void Print(HDC hdc);
	void ReColorBall();
};

extern struct Self* Me;