
#ifndef		ITEM_HPP
# define	ITEM_HPP

# include <iostream>

class Item
{
	public:
		Item(void);
		Item(Item const & src);
		~Item(void);

		Item & operator = (Item const & rhs);

	private:
};

std::ostream			&operator<<(std::ostream &o, Item const &i);

#endif
