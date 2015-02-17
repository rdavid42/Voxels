
#ifndef		BLOCKITEM_HPP
# define	BLOCKITEM_HPP

# include <iostream>
# include "Item.hpp"
# include "Vec3.hpp"

class BlockItem : public Item
{
	public:
		BlockItem(void);
		BlockItem(Vec3<float> color);
		BlockItem(BlockItem const & src);
		~BlockItem(void);


		Vec3<float>		color;

		BlockItem & operator = (BlockItem const & rhs);

	private:
};

std::ostream			&operator<<(std::ostream &o, BlockItem const &i);

#endif
