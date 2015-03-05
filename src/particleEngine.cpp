
#include "particleEngine.hpp"

particleEngine::particleEngine(void)
{
	return ;
}

particleEngine::particleEngine(particleEngine const & src)
{
	*this = src;

	return ;
}

particleEngine::~particleEngine(void)
{
	return ;
}

bool
checkDeath(Particle *toCheck)
{
	if (toCheck->isDead())
	{
		delete toCheck;
		return (1);
	}
	else
		return (0);
}

void
particleEngine::cleanDead(void)
{
	this->particleList.remove_if(checkDeath);
	return ;
}

void
particleEngine::blockDestruction(Vec3<float> pos, Vec3<float> color)
{
	Particle		*tmp;
	int				particleNumber;
	int				i = 0;

	particleNumber = random() % 5;
	while (i <= particleNumber)
	{
		tmp = new Particle;
		tmp->initForDestruction(pos, color);
		this->particleList.push_back(tmp);
		i++;
	}
	return ;
}

void
particleEngine::particleGestion(void)
{

	this->cleanDead();

	std::list <Particle *>::iterator p = this->particleList.begin();
	while (p != this->particleList.end())
	{
		(*p)->run();
		++p;
	}
}

particleEngine &
particleEngine::operator=(particleEngine const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, particleEngine const &i)
{
	o << "Class : particleEngine" << std::endl;
	(void)i;
	return (o);
}
