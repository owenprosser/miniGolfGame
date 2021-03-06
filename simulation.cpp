/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include <iostream>


/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
/*
vec2	gPlaneNormal_Left(1.0,0.0);
vec2	gPlaneNormal_Top(0.0,1.0);
vec2	gPlaneNormal_Right(-1.0,0.0);
vec2	gPlaneNormal_Bottom(0.0,-1.0);
*/

table gTable;

static const float gRackPositionX[] = {0.0f,0.0f,(BALL_RADIUS*2.0f),(-BALL_RADIUS*2.0f),(BALL_RADIUS*4.0f)}; 
static const float gRackPositionZ[] = {0.5f,0.0f,(-BALL_RADIUS*3.0f),(-BALL_RADIUS*3.0f)}; 

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;


/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	//work out rack position
	if (index == 0)
	{
		position(1) = 0.5;
		position(0) = 0.0;
		return;
	}

	static const float sep = (BALL_RADIUS*3.0f);
	static const float rowSep = (BALL_RADIUS*2.5f);
	int row = 1;
	int rowIndex = index;
	while (rowIndex > row)
	{
		rowIndex -= row;
		row++;
	}
	position(1) = -(rowSep * (row - 1));
	position(0) = (((row - 1)*sep) / 2.0f) - (sep*(row - rowIndex));
}

void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if (velocity.Magnitude() <= 0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms) / 1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if (velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void ball::DoBallCollision(ball &b)
{
	if (HasHitBall(b)) HitBall(b);
}

void ball::DoPlaneCollision(const cushion &b)
{
	if (HasHitPlane(b)) HitPlane(b);
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	////if moving away from plane, cannot hit
	//if(velocity.Dot(c.normal) >= 0.0) return false;
	//
	////if in front of plane, then have not hit
	//vec2 relPos = position - c.vertices[0];
	//double sep = relPos.Dot(c.normal);


	/*if (sep > radius)
	{
		return false;
	}*/

	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.centre;

	//std::cout << "relpos: " << relPos(0) << std::endl;

	std::string type = "null";
	double temp = 0;

	if (c.vertices[0](0) == c.vertices[1](0) && c.vertices[0](1) != c.vertices[1](1)) {
		temp = c.centre(0);

		double newTemp = (position(0) - temp);
		if (newTemp < 0) {
			newTemp *= -1;
		}
		bool canCollide = false;

		if (newTemp < BALL_RADIUS) {

			if (c.vertices[0](1) - c.vertices[1](1) < 0) {
				if (position(1) > c.vertices[0](1)) {
					if (position(1) < c.vertices[1](1)) canCollide = true;
				}
			}
			else if (c.vertices[0](1) - c.vertices[1](1) > 0) {
				if (position(1) < c.vertices[0](1)) {
					if (position(1) > c.vertices[1](1)) canCollide = true;
				}
			}
			if (canCollide) { //Return true after 
				return true;
			}
			else {
				return false;
			}

		}
		else {
			return false;
		}
	}
	else if (c.vertices[0](1) == c.vertices[1](1) && c.vertices[0](0) != c.vertices[1](0)) {
		temp = c.centre(1);

		double newTemp = (position(1) - temp);
		if (newTemp < 0) {
			newTemp *= -1;
		}
		bool canCollide = false;

		if (newTemp < BALL_RADIUS) {
			if (c.vertices[0](0) - c.vertices[1](0) < 0) {
				if (position(0) > c.vertices[0](0)) {
					if (position(0) < c.vertices[1](0)) canCollide = true;
				}
			}
			else if (c.vertices[0](0) - c.vertices[1](0) > 0) {
				if (position(0) < c.vertices[0](0)) {
					if (position(0) > c.vertices[1](0)) canCollide = true;
				}
			}
			if (canCollide) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	
}

bool ball::HasHitBall(const ball &b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float) relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if(relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if(dist > (radius+b.radius)) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta; 

	////make some particles
	//int n = (rand()%4)+3;
	//vec3 pos(position(0),radius/2.0,position(1));
	//vec3 oset(c.normal(0),0.0,c.normal(1));
	//pos+=(oset*radius);
	//for(int i=0;i<n;i++)
	//{
	//	gTable.parts.AddParticle(pos);
	//}

/*
	//assume elastic collision
	//find plane normal
	vec2 planeNorm = gPlaneNormal_Left;
	//split velocity into 2 components:
	//find velocity component perpendicular to plane
	vec2 perp = planeNorm*(velocity.Dot(planeNorm));
	//find velocity component parallel to plane
	vec2 parallel = velocity - perp;
	//reverse perpendicular component
	//parallel component is unchanged
	velocity = parallel + (-perp)*gCoeffRestitution;
*/
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity-(relDir*perpV);
	vec2 parallelV2 = b.velocity-(relDir*perpV2);
	
	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass-b.mass))/sumMass) + (float)((perpV2*(2.0*b.mass))/sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass-mass))/sumMass) + (float)((perpV*(2.0*mass))/sumMass);
	
	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);


	//make some particles
	/*int n = (rand()%5)+5;
	vec3 pos(position(0),radius/2.0,position(1));
	vec3 oset(relDir(0),0.0,relDir(1));
	pos+=(oset*radius);
	for(int i=0;i<n;i++)
	{
		gTable.parts.AddParticle(pos);
	}*/
}

/*-----------------------------------------------------------
  particle class members
  -----------------------------------------------------------*/
void particle::update(int ms)
{
	position += (velocity*ms)/1000.0;
	velocity(1) -= (4.0*ms)/1000.0; //(9.8*ms)/1000.0;
}

/*-----------------------------------------------------------
  particle set class members
  -----------------------------------------------------------*/
void particleSet::AddParticle(const vec3 &pos)
{
	if(num >= MAX_PARTICLES) return;
	particles[num] = new particle;
	particles[num]->position = pos;

	particles[num]->velocity(0) = ((rand() % 200)-100)/200.0;
	particles[num]->velocity(2) = ((rand() % 200)-100)/200.0;
	particles[num]->velocity(1) = 2.0*((rand() % 100)/100.0);

	num++;
}

void particleSet::update(int ms)
{
	int i=0;
	while(i<num)
	{
		particles[i]->update(ms);
		if((particles[i]->position(1) < 0.0) && (particles[i]->velocity(1)<0.0))
		{
			delete particles[i];
			particles[i] = particles[num-1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
void table::SetupCushions(void)
{
	double hole1[52]{
			-1, 1, 1, 1,
			1, 1, 1, 0.5,
			1, 0.5, 0.5, 0.5,
			0.5, 0.5, 0.5, -1,
			0.5, -1, 1, -1,
			1, -1, 1, -1.5,
			1, -1.5, -1, -1.5,
			-1, -1.5, -1, -1,
			-1, -1, -0.5, -1,
			-0.5, -1, -0.5, 0.5,
			-0.5, 0.5, -1, 0.5,
			-1, 0.5, -1, 1
	};

	double hole2[32]{
			-1, -1, -1, 1, //LEFT WALL
			-1, 1, 1, 1, //BOTTOM WALL
			1, 1, 1, -1, //RIGHT WALL
			1, -1, 0.5, -1,
			0.5, -1, 0.5, -2,
			0.5, -2, -0.5, -2,
			-0.5, -2, -0.5, -1,
				-0.5, -1, -1, -1
	};

	double hole3[32]{
		-2, -2, -2, 2,
		-2, 2, 2, 2,
		2, 2, 2, -2,
		2, -2, -2, -2,

	   -1,1,-1,-1,
	   -1,-1,1,-1,
	   1,-1,1,1,
	   1,1,-1,1
	};

	double hole4[32]{
		-1, 1, 0, 1,
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 0, 1, 1,
		1, 1, 2, 1,
		2, 1, 2, -1.5,
		2, -1.5, -1, -1.5,
		-1, -1.5, -1, 1
	};

	int count = 0;
	for(int i = 0; i < 12; i++) // Hole 1
	{
		cushions[i].vertices[0](0) = hole1[count++]*2;
		cushions[i].vertices[0](1) = hole1[count++]*2;
		cushions[i].vertices[1](0) = hole1[count++]*2;
		cushions[i].vertices[1](1) = hole1[count++]*2;
	}
	count = 0;

	for (int i = 12; i < 20; i++) //Hole 2
	{
		cushions[i].vertices[0](0) = hole2[count++]*2 + 5;
		cushions[i].vertices[0](1) = hole2[count++]*2;
		cushions[i].vertices[1](0) = hole2[count++]*2 + 5;
		cushions[i].vertices[1](1) = hole2[count++]*2;
	}
	count = 0;

	for (int i = 20; i < 28; i++) //Hole 3
	{
		cushions[i].vertices[0](0) = hole3[count++]-1;
		cushions[i].vertices[0](1) = hole3[count++]-6.5;
		cushions[i].vertices[1](0) = hole3[count++]-1;
		cushions[i].vertices[1](1) = hole3[count++]-6.5;
	}
	count = 0;

	for (int i = 28; i < 36; i++) //Hole 4
	{
		cushions[i].vertices[0](0) = hole4[count++]*2 + 4;
		cushions[i].vertices[0](1) = hole4[count++]*2 - 6.5;
		cushions[i].vertices[1](0) = hole4[count++]*2 + 4;
		cushions[i].vertices[1](1) = hole4[count++]*2 - 6.5;
	}

	/*gTable.balls[0].position(0) = 0;
	gTable.balls[0].position(1) = -3.5;*/

	for(int i=0;i<NUM_CUSHIONS;i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}

	for (int i = 0; i < NUM_PLAYERS; i++) {
		player newPlayer;
		newPlayer.playerNum = i;
		newPlayer.position(0) = 0;
		newPlayer.position(1) = 0.5;

		gTable.players.push_back(newPlayer);
	}

	//Setup Holes
	double holeXPos[4] = { 1.5, 5, -1, 7 };
	double holeYPos[4] = { -2.5, -3, -8, -6 };

	for (int i = 0; i < 4; i++) {
		gTable.holes[i].position(0) = holeXPos[i];
		gTable.holes[i].position(1) = holeYPos[i];
	}
}

void table::Update(int ms)
{
	//check for collisions for each ball
	for(int i=0;i<NUM_BALLS;i++) 
	{
		for(int j=0;j<NUM_CUSHIONS;j++)
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for(int j=(i+1);j<NUM_BALLS;j++) 
		{
			balls[i].DoBallCollision(balls[j]);
		}
	}
	
	//update all balls
	for(int i=0;i<NUM_BALLS;i++) balls[i].Update(ms);

	//update particles
	parts.update(ms);

	//make some new particles
	//vec3 pos(0.0,BALL_RADIUS,0.0);
	//parts.AddParticle(pos);
}

void table::ManagePositions(void) {
	if (gTable.currentPlayer >= NUM_PLAYERS) gTable.currentPlayer = 0;

	gTable.players[currentPlayer].position(0) = gTable.balls[0].position(0);
	gTable.players[currentPlayer].position(1) = gTable.balls[0].position(1);

	gTable.players[currentPlayer].strokes++;

	cout << "Current Player: " << currentPlayer << " Strokes: " << gTable.players[currentPlayer].strokes << endl << endl;

	gTable.currentPlayer++;
	cout << "Managed Position: " << gTable.balls[0].position(0);
}

void table::MoveBall(void){
	if (gTable.currentPlayer >= NUM_PLAYERS) gTable.currentPlayer = 0;
	if (gTable.players[currentPlayer].currentHoleComplete == false) {
		gTable.balls[0].velocity(0) = 0;
		gTable.balls[0].velocity(1) = 0;
		gTable.balls[0].position(0) = gTable.players[currentPlayer].position(0);
		gTable.balls[0].position(1) = gTable.players[currentPlayer].position(1);
	}
	cout << "Move BALL: " << gTable.balls[0].position(0);
}

void table::CheckHoles(void){
	
	for (int i = 0; i < 4; i++) {
		vec2 mag = gTable.balls[0].position - gTable.holes[i].position;
		float dist = (float)mag.Magnitude();
		//cout << "\n\n MAG: "<< dist << "\n";

		if (dist < BALL_RADIUS*5) {
			//cout << "IN HOLE";
			gTable.players[currentPlayer].currentHoleComplete = true;

			int playersCompletedCurrentHole = 0;

			for (int i = 0; i < NUM_PLAYERS; i++) {
				if (gTable.players[i].currentHoleComplete == true) {
					playersCompletedCurrentHole++;
				}
			}
			//cout << "\n players completed hole: " << playersCompletedCurrentHole;
			if (playersCompletedCurrentHole == NUM_PLAYERS) {
				NextHole();
			}
		}
	}

}

void table::NextHole(void) {

	cout << "\nmoving to next hole \n";
	gTable.balls[0].velocity(0) = 0;
	gTable.balls[0].velocity(1) = 0;
	gTable.balls[0].position(0) = gTable.spawnXPos[gTable.currentHole + 1];
	gTable.balls[0].position(1) = gTable.spawnYPos[gTable.currentHole + 1];
	cout << "\n\n Current Hole; " << currentHole;
	cout << "\n\npositions: " << spawnXPos[currentHole + 1] << "   " << spawnYPos[currentHole + 1];

	gTable.ManagePositions();

	for (int i = 0; i < NUM_PLAYERS; i++) {
		gTable.players[i].currentHoleComplete = false;
	}

	gTable.currentHole++;
}

bool endOfSession = false; 

bool table::AnyBallsMoving(void) const
{ //
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if (balls[i].velocity(0) != 0.0 || balls[i].velocity(1) != 0.0) {
			endOfSession = true;
			gTable.CheckHoles();
			return true;
		}
	}

	if (endOfSession == true) {
		cout << "running manage positions()";
		gTable.ManagePositions();
		gTable.MoveBall();
	}

	endOfSession = false;

	return false;
}