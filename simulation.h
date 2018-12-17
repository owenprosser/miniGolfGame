/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include <iostream>
#include"vecmath.h"

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X			(0.6f) 
#define TABLE_Z			(1.2f)
#define BALL_RADIUS		(0.05f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)
#define NUM_BALLS		(1)		

/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;

/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;

	ball(): position(0.0), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset();}
	
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollisions(void);
	void DoBallCollision(ball &b);
	void Update(int ms);
	
	bool HasHitPlane1(void) const;
	bool HasHitPlane2(void) const;
	bool HasHitPlane3(void) const;
	bool HasHitPlane4(void) const;
	bool HasHitBall(const ball &b) const;

	void HitPlane1(void);
	void HitPlane2(void);
	void HitPlane3(void);
	void HitPlane4(void);
	void HitBall(ball &b);
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	ball balls[NUM_BALLS];	
	void Update(int ms);	
	bool AnyBallsMoving(void) const;

	int currentLevel = 0;
	float tableArray[4][3]= {
	    {-0.6f, 0.0, -1.2f} , 
		{-0.6f, 0.1, -1.2f} ,
		{-0.6f, 0.1, 1.2f} ,
		{-0.6f, 0.0, 1.2f}
	};
};

class player
{
public:
	int score = 0;
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern table gTable;
extern player player1;
extern player player2;