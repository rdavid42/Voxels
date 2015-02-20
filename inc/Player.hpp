
#ifndef		PLAYER_HPP
# define	PLAYER_HPP

# include <iostream>
# include "Container.hpp"

class Player
{
	public:
		Player(void);
		Player(Player const & src);
		~Player(void);

		std::string		name;
		Container *		inventory;
		bool			creative;

		void			changeMode(void);

		Player & operator = (Player const & rhs);

	private:
};

std::ostream			&operator<<(std::ostream &o, Player const &i);

#endif
