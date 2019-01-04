/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>
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

	if (c.vertices[0](0) == c.vertices[1](0) && c.vertices[0](1) != c.vertices[1](1)) {
		type = "vertical";
	}
	else if (c.vertices[0](1) == c.vertices[1](1) && c.vertices[0](0) != c.vertices[1](0)) {
		type = "horizontal";
	}


	double tempVal = 0;
	if (type == "vertical") {
		tempVal = c.centre(0);

		double newTemp = (position(0) - tempVal);
		if (newTemp < 0) {
			newTemp *= -1;
		}
		bool canCollide = false;

		if (newTemp < 0.05) {

			if (c.vertices[0](1) - c.vertices[1](1) < 0) {
				if (position(1) > c.vertices[0](1) && position(1) < c.vertices[1](1)) {   //IF Y POSITION IS GREATER THAN FIRST VERTEX Y POS, AND LESS THAN SECOND VERTEX Y POS, CAN COLLIDE
			//		std::cout << "vertical" << std::endl;
					canCollide = true;
				}
			}
			else if (c.vertices[0](1) - c.vertices[1](1) > 0) {
				if (position(1) < c.vertices[0](1) && position(1) > c.vertices[1](1)) {   //IF Y POSITION IS less THAN FIRST VERTEX Y POS, AND greater THAN SECOND VERTEX Y POS, CAN COLLIDE
		//			std::cout << "vertical2" << std::endl;
					canCollide = true;
				}
			}

			if (canCollide) {
				//			std::cout << "cancollide" << std::endl;
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

	else if (type == "horizontal") {
		tempVal = c.centre(1);

		double newTemp = (position(1) - tempVal);
		if (newTemp < 0) {
			newTemp *= -1;
		}
		bool canCollide = false;

		if (newTemp < 0.05) {
			//std::cout << "pos: " << position(0) << "tempVal: " << tempVal << std::endl;
			if (c.vertices[0](0) - c.vertices[1](0) < 0) {
				if (position(0) > c.vertices[0](0) && position(0) < c.vertices[1](0)) {   //IF Y POSITION IS GREATER THAN FIRST VERTEX Y POS, AND LESS THAN SECOND VERTEX Y POS, CAN COLLIDE
			//		std::cout << "horizontal" << std::endl;
					canCollide = true;
				}
			}
			else if (c.vertices[0](0) - c.vertices[1](0) > 0) {
				if (position(0) < c.vertices[0](0) && position(0) > c.vertices[1](0)) {   //IF Y POSITION IS GREATER THAN FIRST VERTEX Y POS, AND LESS THAN SECOND VERTEX Y POS, CAN COLLIDE
	//				std::cout << "horizontal2" << std::endl;
					canCollide = true;
				}
			}

			if (canCollide) {
				//		std::cout << "cancollide" << std::endl;
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

	double table1[52]{
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

	double table2[32]{
					-1, -1, -1, 1, //LEFT WALL
					-1, 1, 1, 1, //BOTTOM WALL
					1, 1, 1, -1, //RIGHT WALL
					1, -1, 0.5, -1,
					0.5, -1, 0.5, -2,
					0.5, -2, -0.5, -2,
					-0.5, -2, -0.5, -1,
					 -0.5, -1, -1, -1
	};

	double table3[32]{
		-2, -2, -2, 2,
		-2, 2, 2, 2,
		2, 2, 2, -2,
		2, -2, -2, -2,

	   -1,1,-1,-1,
	   -1,-1,1,-1,
	   1,-1,1,1,
	   1,1,-1,1
	};

	double table4[32]{
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
		cushions[i].vertices[0](0) = table1[count++]*2;
		cushions[i].vertices[0](1) = table1[count++]*2;
		cushions[i].vertices[1](0) = table1[count++]*2;
		cushions[i].vertices[1](1) = table1[count++]*2;
	}
	count = 0;

	for (int i = 12; i < 20; i++) //Hole 2
	{
		cushions[i].vertices[0](0) = table2[count++]*2 + 5;
		cushions[i].vertices[0](1) = table2[count++]*2;
		cushions[i].vertices[1](0) = table2[count++]*2 + 5;
		cushions[i].vertices[1](1) = table2[count++]*2;
	}
	count = 0;

	for (int i = 20; i < 28; i++) //Hole 3
	{
		cushions[i].vertices[0](0) = table3[count++]-1;
		cushions[i].vertices[0](1) = table3[count++]-6.5;
		cushions[i].vertices[1](0) = table3[count++]-1;
		cushions[i].vertices[1](1) = table3[count++]-6.5;
	}
	count = 0;

	for (int i = 28; i < 36; i++) //Hole 4
	{
		cushions[i].vertices[0](0) = table4[count++]*2 + 4;
		cushions[i].vertices[0](1) = table4[count++]*2 - 6.5;
		cushions[i].vertices[1](0) = table4[count++]*2 + 4;
		cushions[i].vertices[1](1) = table4[count++]*2 - 6.5;
	}


	/*gTable.balls[0].position(0) = 0;
	gTable.balls[0].position(1) = -3.5;*/

	for(int i=0;i<NUM_CUSHIONS;i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
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

bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if(balls[i].velocity(0)!=0.0) return true;
		if(balls[i].velocity(1)!=0.0) return true;
	}
	return false;
}
