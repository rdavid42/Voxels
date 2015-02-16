#ifndef CONSTANTS_HPP
# define CONSTANTS_HPP

# define MASK_1			0x00000001
# define MASK_2			0x00000003
# define MASK_3			0x00000007
# define MASK_4			0x0000000F
# define MASK_5			0x0000001F
# define MASK_6			0x0000003F
# define MASK_7			0x0000007F
# define MASK_8			0x000000FF
# define MASK_9			0x000001FF
# define MASK_10		0x000003FF
# define MASK_11		0x000007FF
# define MASK_12		0x00000FFF
# define MASK_13		0x00001FFF
# define MASK_14		0x00003FFF
# define MASK_15		0x00007FFF
# define MASK_16		0x0000FFFF
# define MASK_17		0x0001FFFF
# define MASK_18		0x0003FFFF
# define MASK_19		0x0007FFFF
# define MASK_20		0x000FFFFF
# define MASK_21		0x001FFFFF
# define MASK_22		0x003FFFFF
# define MASK_23		0x007FFFFF
# define MASK_24		0x00FFFFFF
# define MASK_25		0x01FFFFFF
# define MASK_26		0x03FFFFFF
# define MASK_27		0x07FFFFFF
# define MASK_28		0x0FFFFFFF
# define MASK_29		0x1FFFFFFF
# define MASK_30		0x3FFFFFFF
# define MASK_31		0x7FFFFFFF
# define MASK_32		0xFFFFFFFF

// Octree states
# define EMPTY				0
# define GROUND				1
# define CHUNK				2

# define OCTREE_SIZE		20000.0f

# define CHUNK_DEPTH		15 // insert directly in octree
# define BLOCK_DEPTH		5 // insert from chunk depth
# define PLANT_DEPTH		8 // insert from chunk depth
// [1, 3, 5, 7, 9, ...]
# define GEN_SIZE			13 // (n * n * n) must be odd in order to place camera in the center all the time

#endif
