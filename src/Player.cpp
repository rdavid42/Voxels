
#include "Player.hpp"

Player::Player(void)
{
	std::cout << "player crée" << std::endl;
	this->inventory = new Container;
	return ;
}

Player::Player(Player const & src)
{
	*this = src;

	return ;
}

Player::~Player(void)
{
	return ;
}

Player &
Player::operator=(Player const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, Player const &i)
{
	o << "Class : Player" << std::endl;
	(void)i;
	return (o);
}
