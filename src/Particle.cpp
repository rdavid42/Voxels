
#include "Particle.hpp"

Particle::Particle(void)
{
	float		velX;
	float		velY;
	float		testX;

	velX = (double) random() / RAND_MAX;
	velY = (double) random() / RAND_MAX;
	velX /= 10;
//	velY /= 10;
	if (random() % 2 == 1)
		velX = -velX;
	testX = (double) random() / RAND_MAX;
	this->_lifeSpan = 100;
	/*
	this->_color = Vec3<float> (0.0f, 0.0f, 0.0f);
	this->_endColor = Vec3<float> (0.0f, 0.0f, 0.0f);
	this->_deltaColor = Vec3<float>
		(((this->_endColor.x - this->_color.x) / this->_lifeSpan),
		((this->_endColor.y - this->_color.y) / this->_lifeSpan),
		((this->_endColor.z - this->_color.z) / this->_lifeSpan));
	this->_position = Vec3<float> (testX, 0.0f, 0.0f);
	this->_velocity = Vec3<float> (velX , velY, 0.0f);
	this->_acceleration = Vec3<float> (0.0f, -0.01f, 0.0f);
*/	return ;

}

Particle::Particle(Particle const & src)
{
	*this = src;

	return ;
}

void
Particle::run(void)
{
	this->update();
	if (!this->isDead())
		this->drawTriangle();
	return ;
}

void
Particle::initForDestruction(Vec3<float> &pos, Vec3<float> &color)
{
	float		velX;
	float		velY;
	float		velZ;
	float		randColorX;
	float		randColorY;
	float		randColorZ;

	randColorX = ((double) random() / RAND_MAX) / 10;
	randColorY = ((double) random() / RAND_MAX) / 10;
	randColorZ = ((double) random() / RAND_MAX) / 10;
	randColorX -= ((double) random() / RAND_MAX) / 10;
	randColorY -= ((double) random() / RAND_MAX) / 10;
	randColorZ -= ((double) random() / RAND_MAX) / 10;
	std::cout << randColorX << std::endl;
	std::cout << randColorY << std::endl;
	std::cout << randColorZ << std::endl;
	this->_position = pos;
	this->_color = Vec3<float> (color.x + randColorX,
								color.y + randColorY,
								color.z + randColorZ);
	this->_endColor = this->_color;
	this->_deltaColor = Vec3<float> (0.0f, 0.0f, 0.0f);
	velX = ((double) random() / RAND_MAX) / 10;
	velY = ((double) random() / RAND_MAX) / 10;
	velZ = ((double) random() / RAND_MAX) / 10;
	this->_velocity = Vec3<float> (velX, velY, velZ);
	this->_acceleration = Vec3<float>(0.0f, 0.0f, -0.1f);
	this->_lifeSpan = 50;
	return ;
}

void
Particle::initwithPos(float x, float y, float z)
{
	float		velX;
	float		velY;
	float		velZ;


	this->_position = Vec3<float> (x, y, z);
	velX = (double) random() / RAND_MAX;
	velY = (double) random() / RAND_MAX;
	velZ = (double) random() / RAND_MAX;
	if (x < 0.0f)
		velX = -velX;
	if (z < 0.0f)
		velZ = -velZ;
//	velX /= 10;
//	velY /= 10;
//	velZ /= 10;
	this->_lifeSpan = 100;
	this->_acceleration = Vec3<float> (0.0f, 0.0f, 0.0f);
}

void
Particle::drawTriangle(void)
{
	int		i = 0;
	int		triangleAmount = 20;
	float	radius = 0.1;
	GLfloat	twicePi = 2.0f * M_PI;

	glColor4f(this->_color.x, this->_color.y, this->_color.z, this->_lifeSpan / 100);
	glPushMatrix();
		glTranslatef(this->_position.x, this->_position.y, this->_position.z);
		glBegin(GL_TRIANGLES);

		//TRIANGLE BAS
		glVertex3f( 0.0f, 0.005f, 0.0f);
		glVertex3f(-0.005f,-0.005f, 0.005f);
		glVertex3f( 0.005f,-0.005f, 0.005f);
		//TRIANGLE DROIT
		glVertex3f(0.0f, 0.005f, 0.0f);
		glVertex3f(0.005f, -0.005f, 0.005f);
		glVertex3f(0.005f, -0.005f, -0.005f);

		//TRIANGLE ARRIRE
		glVertex3f(0.0f, 0.005f, 0.0f);
		glVertex3f(0.005f, -0.005f, -0.005f);
		glVertex3f(-0.005f, -0.005f, -0.005f);

		//TRIANGLE GAUCHE
		glVertex3f( 0.0f, 0.005f, 0.0f);
		glVertex3f(-0.005f, -0.005f, -0.005f);
		glVertex3f(-0.005f, -0.005f, 0.005f);

		glEnd();
	glPopMatrix();
}

void
Particle::update(void)
{
	this->_velocity += this->_acceleration;
	this->_position += this->_velocity;
	this->_lifeSpan -= 1.0f;
	this->_color += this->_deltaColor;
}

bool
Particle::isDead(void)
{
	if (this->_lifeSpan < 1.0f)
		return true;
	else
		return false;
}

Vec3<float>
Particle::getPosition(void) const
{
	return (this->_position);
}

void
Particle::setPosition(Vec3<float> &newPos)
{
	this->_position = newPos;
	return ;
}


Vec3<float>
Particle::getVelocity(void) const
{
	return (this->_velocity);
}

void
Particle::setVelocity(Vec3<float> &newVelocity)
{
	this->_velocity = newVelocity;
	return ;
}


Vec3<float>
Particle::getAcceleration(void) const
{
	return (this->_acceleration);
}

void
Particle::setAcceleration(Vec3<float> &newAcceleration)
{
	this->_acceleration = newAcceleration;
	return ;
}

float
Particle::getLifeSpan(void) const
{
	return (this->_lifeSpan);
}

void
Particle::setLifeSpan(int &newLifeSpan)
{
	this->_lifeSpan = newLifeSpan;
	return ;
}

Particle::~Particle(void)
{
	return ;
}

Particle &
Particle::operator=(Particle const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, Particle const &i)
{
	o << "Class : Particle" 
		<< "Position :" << i.getPosition() << "\n"
		<< "Velocity :" << i.getVelocity() << "\n"
		<< "Acceleration :" << i.getAcceleration() << "\n"

		<<std::endl;
	return (o);
}
