
#ifndef		CONTAINER_HPP
# define	CONTAINER_HPP

# include <iostream>
# include "Item.hpp"
# include "Vec3.hpp"
# include "BlockItem.hpp"

class Container
{
	public:
		Container(void);
		Container(Container const & src);
		~Container(void);

		Container & operator = (Container const & rhs);

		BlockItem			*stock[20];
		int					selected;

		void			add(BlockItem const & item);
		void			selectItem(int selection);
		void			deleteFirst(void);
		void			deleteSelected(void);
		void			drawInventory(void);
		Vec3<float>		getFirstBlock(void);

	private:
};

std::ostream			&operator<<(std::ostream &o, Container const &i);

#endif
