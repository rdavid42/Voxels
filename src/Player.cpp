
#include "Player.hpp"
#include "Core.hpp"

Player::Player(void)
{
	std::cout << "player crée" << std::endl;
	this->inventory = new Container();
	this->creative = false;
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

void
Player::changeMode(void)
{
	if (this->creative == false)
	{
		this->creative = true;
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_ShowCursor(SDL_ENABLE);
	}
	else
	{
		this->creative = false;
		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_ShowCursor(SDL_DISABLE);
	}
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
