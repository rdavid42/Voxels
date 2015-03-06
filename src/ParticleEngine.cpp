
#include "ParticleEngine.hpp"

ParticleEngine::ParticleEngine(void)
{
	return ;
}

ParticleEngine::ParticleEngine(ParticleEngine const & src)
{
	*this = src;

	return ;
}

ParticleEngine::~ParticleEngine(void)
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
ParticleEngine::cleanDead(void)
{
	this->particleList.remove_if(checkDeath);
	return ;
}

void
ParticleEngine::blockDestruction(Vec3<float> pos, Vec3<float> color)
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
ParticleEngine::particleGestion(void)
{

	this->cleanDead();

	std::list <Particle *>::iterator p = this->particleList.begin();
	while (p != this->particleList.end())
	{
		(*p)->run();
		++p;
	}
}

ParticleEngine &
ParticleEngine::operator=(ParticleEngine const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, ParticleEngine const &i)
{
	o << "Class : ParticleEngine" << std::endl;
	(void)i;
	return (o);
}
