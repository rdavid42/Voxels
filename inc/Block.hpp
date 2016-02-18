
#ifndef		BLOCK_HPP
# define	BLOCK_HPP

# include <iostream>
# include "Constants.hpp"

class Core;

class Block
{
public:
	Block(void);
	~Block(void);

	void				setType(uint8_t const &type);
	uint8_t const &		getType(void) const;

	Block				&operator=(Block const &rhs);

private:
	Block(Block const &src);

	uint8_t				_type;
};

std::ostream			&operator<<(std::ostream &o, Block const &i);

#endif
