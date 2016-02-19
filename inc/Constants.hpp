#ifndef CONSTANTS_HPP
# define CONSTANTS_HPP

# if defined(__APPLE_CC__)
#  ifndef GLFW_INCLUDE_GLCOREARB
#   define GLFW_INCLUDE_GLCOREARB
#  endif
#  ifndef GLFW_INCLUDE_GLEXT
#   define GLFW_INCLUDE_GLEXT
#  endif
# else
#  define GL_GLEXT_PROTOTYPES
# endif

# include <GLFW/glfw3.h>

# define MASK_1						0x00000001

# define CHUNK_SIZE					32
# define BLOCK_SIZE					1.0

# define GEN_SIZE_X					15
# define GEN_SIZE_Y					8
# define GEN_SIZE_Z					15

// Noises bounds
# define TASK_QUEUE_OVERFLOW		100000

typedef enum
{
	AIR,
	SIDE_GRASS,
	DIRT,
	GRASS,
	STONE,
	COAL,
	LEAF,
	WOOD
}					block_type;

#endif
