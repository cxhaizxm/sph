#include "fluid.h"
#include <iostream>

//Globals
const double h = 0.1;
const double containerSizeX = 10.0; 
const double containerSizeY = 10.0; 
const double containerSizeZ = 10.0; 
const double k = 300; //TODO - make this function dependant on the temp

const double PI = 3.14159265; 

Fluid::Fluid(void)// : container( 100, 100, 100, vec3( -3, 0, -3 ), vec3( 3, 6, 3 ) )
{
	frame = 0;
	/*srand (time(NULL));*/
	//Sets up the size of the grid that the particles fall into 
	SetGridSize(containerSizeX, containerSizeY, containerSizeZ);

	/*
	
	//*/
}


Fluid::~Fluid(void)
{
}

//Draws the current frame 
void Fluid::Draw(const glm::vec3& eyePos)
{

}

    // Reset to the initial state
void Fluid::Reset()
{

}

//Initialize fluid from some point
void Fluid::addFluid(float dt)
{
	//Density, mass, position, velocity (particle inputs)
	/*
	for (float z = -0.2; z < 0.2; z+=0.1)
    {
		for (float y = 1.8; y < 2.2; y += 0.1 )
		{
			float rx = 0.01*((double) rand() / (RAND_MAX)); 
			float ry = 0.01*((double) rand() / (RAND_MAX)); 
			float rz = 0.01*((double) rand() / (RAND_MAX)); 

			//Density, mass, position, velocity (particle inputs)
			Particle * p = new Particle(1500, 1, glm::vec3(rx-1.5, y+ry, z+rz), glm::vec3(-3, 0.0, 0.0));
			theParticles.push_back(p);
		}
	}
	/*/
	for( float y = 0; y < 2; y += 0.1 )
	{
		for( float x = -0.5; x < 0.5; x += 0.1 )
		{
			for( float z = -0.5; z < 0.5; z += 0.1 )
			{
				float rx = 0.01*(float)rand() / RAND_MAX; 
				float ry = 0.01*(float)rand() / RAND_MAX; 
				float rz = 0.01*(float)rand() / RAND_MAX; 
				vec3 pos(x+rx, y+ry, z+rz);
				Particle * p = new Particle(1500, 1, pos, glm::vec3(0));
				theParticles.push_back(p);
				//Box * box = container( pos );
				//if( box->frame < frame )
				//{
				//	box->particles.clear();
				//	box->frame = frame;
				//}
				//box->particles.push_back( p );
			}
		}
	}
	//*/
}

//**********************************************************************************************
//The main fluid simulation 
//**********************************************************************************************

//Calls all the SPH fns
void Fluid::Update(float dt, glm::vec3& externalForces)
{
	if (theParticles.size() < 100 )//0 && frame % 15 == 0)
		addFluid(dt);
	findNeighbors();
	computeDensity(dt);
	computeForces(dt, externalForces);
	integrate(dt); 
	//computeVelocity(dt);
	//computePosition(dt);
	resolveCollisions();
	frame++;
	/*
	container.clear();
	for (int i = 0; i < theParticles.size(); i++)
	{
		Particle * p = theParticles.at(i);
		Box * box = container( p->getPosition() );
		box->particles.clear();
		box->frame = frame;
		box->particles.push_back( p );
	}
	*/
}

//Finds all the the particles current neighbors and stores them in the particle's neighbors vector
void Fluid::findNeighbors()
{
	for (int i = 0; i < theParticles.size(); i++)
	{
		Particle * p = theParticles.at(i);
		p->clearNeighbors();
		/*
		vec3 offset( 0 );
		Box * visited[27];
		int j = 0;
		for ( int x = -1; x <= 1; x ++ )
		{
			offset.x = x*h;
			for ( int y = -1; y <= 1; y ++ )
			{
				offset.y = y*h;
				for ( int z = -1; z <= 1; z ++ )
				{
					offset.z = z*h;
					Box * box = container( p->getPosition() + offset );
					visited[j] = box;
					if( box != NULL )
					{
						if( box->frame < frame )
						{
							box->frame = frame;
							box->particles.clear();
						}
						else
						{
							bool flag = true;
							for( int k = 0; k < j; k ++ )
							{
								flag = flag && ( visited[k] != box );
							}
							if( flag )
							{
								p->addNeighbors( box->particles );
							}
						}
					}
					j++;
				}
			}
		}
		/*/
		for (int j = 0 ; j < theParticles.size(); j++)
		{
			//Compute the distance from particle i to j & add it if its within the kernal radius
			if (glm::distance(theParticles.at(i)->getPosition(), theParticles.at(j)->getPosition()) <= h ) {
				theParticles.at(i)->addNeighbor( theParticles.at(j) ); 
			}
		}
		//*/
	}
}

//Finds the density at the current timestep
void Fluid::computeDensity(float dt)
{
	for (int i = 0; i < theParticles.size(); i++)
	{
		float density = 0; //theParticles.at( i ).getRestDensity();
		vec3 pos_i = theParticles.at(i)->getPosition();
		std::vector<Particle*> neighbors = theParticles.at(i)->getNeighbors();
		for (int j = 0; j < neighbors.size(); j++)
		{
			float r = glm::distance(pos_i, neighbors.at(j)->getPosition()); 
			density += (neighbors.at(j)->getMass() * wPoly6(r, h));
		}
		theParticles.at(i)->setDensity(density); 
	}
}

//Computes the current pressure of the particles
//TODO - check.  The paper doesn't store this, but uses it directly in the forces (noted in particle.h as well)
glm::vec3 Fluid::computePressure(float dt, int i )
{
	//Get the pressure: p = k * (currDens - restDens)
	glm::vec3 pressure(0.0);
	float pi = (k * (theParticles.at(i)->getDensity() - theParticles.at(i)->getRestDensity())); 
	std::vector<Particle*> neighbors = theParticles.at(i)->getNeighbors();
	for (int j = 0; j < neighbors.size(); j++) {
		float pj = (k * (neighbors.at(j)->getDensity() - neighbors.at(j)->getRestDensity())); 
		vec3 r = theParticles.at(i)->getPosition() - neighbors.at(j)->getPosition(); 
		//fPressure = - sum (mj (tempPi + tempPj) / 2 pj * gradient(W(ri - rj, h))
		pressure += neighbors.at(j)->getMass() * ((pi + pj) / (1e-15f + 2.0f * neighbors.at(j)->getDensity())) * wSpikyGrad(r, h); 
	}
	float sentinel = glm::length( pressure );
	return -pressure;
}

glm::vec3 Fluid::computeViscosity(float dt, int i)
{
	glm::vec3 v(0.0); 
	std::vector<Particle*> neighbors = theParticles.at(i)->getNeighbors();
	for (int j = 0; j < neighbors.size(); j++)
	{
		glm::vec3 r = theParticles.at(i)->getPosition() - neighbors.at(j)->getPosition();
		glm::vec3 vel = (neighbors.at(j)->getVelocity() - theParticles.at(i)->getVelocity()) / neighbors.at(j)->getDensity();
		v += neighbors.at(j)->getMass()*vel*wViscosityLap( r, h );
	}
	return 15.f*v; 
}

glm::vec3 Fluid::computeSurfaceTension(float dt, int i)
{
	glm::vec3 n(0.0); 
    float k = 0.0; 
	std::vector<Particle*> neighbors = theParticles.at(i)->getNeighbors();
	for (int j = 0; j < neighbors.size(); j++)
	{
		glm::vec3 r = theParticles.at(i)->getPosition() - neighbors.at(j)->getPosition();

		float mass = neighbors.at(j)->getMass(); 
		float denInv = 1.0 / (neighbors.at(j)->getDensity() + 1e-15f); 
		n += mass * denInv * wPoly6Grad(r, h); 
		k += mass * denInv * wPoly6Lap(r, h); 
	}

	k = -k / (glm::length(n) + 1e-15f); 
	n =  n / (glm::length(n) + 1e-15f);
	return 50.f * k * n; 
}


void Fluid::computeForces(float dt, glm::vec3 externalForces)
{
	//TODO - add other forces for now, just add gravity
	for (int i = 0; i < theParticles.size(); i++) 
	{
		glm::vec3 pressureForce = computePressure(dt, i); 
		glm::vec3 viscosityForce = computeViscosity(dt, i); 
		glm::vec3 surfaceTension = computeSurfaceTension(dt, i);
		glm::vec3 finalForce = pressureForce + theParticles.at(i)->getDensity()*externalForces + viscosityForce + surfaceTension; 
		theParticles.at(i)->setForce( finalForce ); 
	}
}

void Fluid::integrate(float dt)
{
	//Euler just in case leapfrog is wrong
	for (int i = 0; i < theParticles.size(); i++)
	{
		Particle * p = theParticles.at(i); 
		p->setVelocity(p->getVelocity() + dt * p->getForce() / p->getDensity());
		p->setPostion(p->getPosition() + dt * p->getVelocity());
	}

	////************Leap frog start **************************
	////http://en.wikipedia.org/wiki/Leapfrog_integration	
	//std::vector<glm::vec3> currAccel;
	//for (int i = 0; i < theParticles.size(); i++)
	//{
	//		Particle& p = *theParticles.at(i); 
	//		glm::vec3 accel = p.getForce() / p.getDensity(); 
	//		currAccel.push_back(accel);
	//		glm::vec3 newPos = p.getPosition() + p.getVelocity() * dt + accel * dt * dt * (float) 0.5;
	//		p.setPostion(newPos); 
	//}
	//
	////std::vector<Particle> particleCopy(theParticles);
	////computeForces(dt, vec3(0, -9.8, 0), particleCopy);
	//std::vector<glm::vec3> newForce; 

	////Update forces
	//findNeighbors(); 
	//computeDensity(dt);
	//for (int i = 0; i < theParticles.size(); i++)
	//{
	//	glm::vec3 pressureForce = computePressure(dt, i); 
	//	glm::vec3 viscosityForce = computeViscosity(dt, i); 
	//	glm::vec3 surfaceTension = computeSurfaceTension(dt, i);
	//	glm::vec3 finalForce = pressureForce + theParticles.at(i)->getDensity()*vec3(0.0, -9.8, 0.0) + viscosityForce + surfaceTension; 
	//	newForce.push_back(finalForce); 
	//}

	//for (int i = 0; i < theParticles.size(); i++)
	//{
	//	vec3 newAccel = newForce.at(i) / theParticles.at(i)->getDensity();
	//	vec3 avgAccel = (float) 0.5 * currAccel.at(i) + newAccel; 
	//	theParticles.at(i)->setVelocity( theParticles.at(i)->getVelocity() + dt * avgAccel);		
	//}
}

void Fluid::computeVelocity(float dt)
{
}

void Fluid::computePosition(float dt)
{

}

void Fluid::resolveCollisions()
{

	// p.velocity = p.velocity - 2.0 * Dot(normal, p.velocity) * normal;
	for (int i = 0; i < theParticles.size(); i++)
	{
		glm::vec3 pos = theParticles.at(i)->getPosition();
		glm::vec3 vel = theParticles.at(i)->getVelocity();
		bool updated = false;
		//For now, don't handle corners
		if (pos.x < -3) {
			glm::vec3 normal = glm::vec3(1, 0, 0); 
			updated = true;
			//Normal of wall is 
			pos.x = -3;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.x *= -1;
		}
		if (pos.y < 0) {
			glm::vec3 normal = glm::vec3(0, 1.0, 0); 
			updated = true;
			pos.y = 0;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.y *= -1;  
		}
		if (pos.z < -3) {
			glm::vec3 normal = glm::vec3(0, 0, 1); 
			updated = true;
			pos.z = -3;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.z *= -1; 
		}

		if (pos.x > 3) {
			glm::vec3 normal = glm::vec3(-1, 0, 0); 
			updated = true;
			pos.x = 3;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.x *= -1;
		}
		if (pos.y > 6) {
			glm::vec3 normal = glm::vec3(0, -1.0, 0); 
			updated = true;
			pos.y = 6;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.y *= -1;  
		}
		if (pos.z > 3) {
			glm::vec3 normal = glm::vec3(0, 0, -1.0); 
			updated = true;
			pos.z = 3;
			vel = vel - (float) 2.0 * glm::dot(normal, vel) * normal; 
			//vel.z *= -1; 
		}

		if (updated)
		{
			theParticles.at(i)->setPostion(pos); 
			theParticles.at(i)->setVelocity(vel);
		}
	}
}


float Fluid::wPoly6(float r, float h)
{
	if (0 <= r && r <= h) {
		float c = 315.0 / (64.0 * PI * pow(h, 9)); 
		float w = pow(pow(h, 2) - pow(r, 2), 3); 
		return c * w; 
	} else {
		return 0.0;
	}
}

glm::vec3 Fluid::wPoly6Grad(glm::vec3 r, float h)
{
	float lr = length( r );
	if (0 <= lr && lr <= h) {
		float lrs = lr*lr-h*h;
		float c = 315.0 / (64.0 * PI * pow(h, 9)); 
		float x = -6*r.x*lrs;
		float y = -6*r.y*lrs;
		float z = -6*r.z*lrs;
		vec3 w(x,y,z); 
		return c * w; 
	} else {
		return vec3( 0.0 );
	}
}

float Fluid::wPoly6Lap(glm::vec3 r, float h)
{
	float c = 315.0 / (64.0 * PI * pow(h, 9)); 
	float lr = glm::length(r); 
	float lrS = lr * lr; 
	if (0 <= lr && lr <= h) {
		float w = -6 * ( 3 * h * h * h * h - 10 * h * h * lrS + 7 * lrS * lrS);
		return c * w; 
	} else {
		return 0.0f; 
	}
}

//Used for pressure calcs
float Fluid::wSpiky(float r, float h)
{
	if (0 <= r && r <= h) {
		float c = 15.0 / (PI * pow(h, 6)); 
		float w = pow (h - r, 3); 
		return c*w;
	} else {
		return 0; 
	}
}
vec3 Fluid::wSpikyGrad(vec3 r, float h)
{
	float lr = length( r );
	if (0 <= lr && lr <= h) {
		float c = 15.0 / (PI * pow(h, 6)); 
		float numerator = pow(lr - h, 2);
		float x = -3*r.x*numerator/(lr+1e-15);
		float y = -3*r.y*numerator/(lr+1e-15);
		float z = -3*r.z*numerator/(lr+1e-15);
		vec3 w = vec3( x, y, z );
		return c*w;
	} else {
		return vec3(0); 
	}
}

float Fluid::wViscosity(float r, float h)
{
	return 0.0f;
}

glm::vec3 Fluid::wViscosityGrad(glm::vec3 r, float h)
{
	return vec3(0.0f);
}

float Fluid::wViscosityLap(glm::vec3 r, float h)
{
	float lr = length( r );
	if (0 <= lr && lr <= h) {
		float c = 45.0f / (PI * pow(h, 6)); 
		float w = h - lr;
		return c*w;
	} else {
		return 0; 
	}
}

//************************************************************************************************
//Grid section 
//TODO - use this for speedup
//************************************************************************************************

//Set/Get our screen resolution
void Fluid::SetGridSize(int cols, int rows, int stacks)
{
	numRows = rows;
	numCols = cols;
	numStacks = stacks; 
}

int Fluid::GetGridCols() const
{
	return numCols;
}
int Fluid::GetGridRows() const
{
	return numRows;
}

int Fluid::GetGridStacks() const
{
	return numStacks; 
}

// Set/Get our draw flags
void Fluid::SetDrawFlags(unsigned int flags)
{
	drawFlags = flags;
}

unsigned int Fluid::GetDrawFlags() const
{
	return drawFlags;
}

const std::vector<Particle*>& Fluid::getParticles()
{
	return theParticles; 
}

