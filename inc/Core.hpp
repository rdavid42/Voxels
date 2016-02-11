
#ifndef CORE_HPP
# define CORE_HPP

# include <thread>
# include <vector>
# include <list>
# include <cmath>
# include <typeinfo>
# include "Camera.hpp"
# include "Shaders.hpp"
# include "Bmp.hpp"
# include "Chunk.hpp"
# include "Block.hpp"
# include "Noise.hpp"

# define STARTED	0
# define STOPPED	1

typedef struct
{
	int				i;
	Core			*core;
}					ThreadArgs;

typedef enum
{
	NONE,
	GRASS,
	STONE,
	COAL,
	LEAF,
	WOOD,
	DIRT
}					block_type;

class Core
{
public:
	/* glfw */
	GLFWwindow				*window;
	int						windowWidth;
	int						windowHeight;

	/* shaders */
	GLuint					vertexShader;
	GLuint					fragmentShader;
	GLuint					program;

	/* matrices */
	Mat4Stack<float>		ms;
	Mat4<float>				projMatrix;
	Mat4<float>				viewMatrix;

	/* Locations */
	GLuint					projLoc;
	GLuint					viewLoc;
	GLuint					objLoc;
	GLuint					positionLoc;
	GLuint					textureLoc;
	GLuint					normalLoc;
	GLuint					colorLoc;
	GLuint					renderVoxelRidgesLoc;

	GLuint					voxelVao;
	GLuint					voxelVbo[2];

	/* Camera */
	Camera					camera;

	/* Textures */
	int						texMax;
	GLuint					*tex;

	std::ostringstream		oss_ticks;

	/* selection */
	GLuint					selectionVao;
	GLuint					selectionVbo[2];
	int						selectionVerticesSize;
	int						selectionIndicesSize;

	/* Octree */

	Noise *					noise;
	Link					*octree;
	float					chunk_size; // size of a chunk
	float					block_size[MAX_BLOCK_DEPTH]; // size of a block inside a chunk
	int						center; // central chunk's index, `chunks[center][center][center]`
	Chunk *					chunks[GEN_SIZE]
								  [GEN_SIZE]
								  [GEN_SIZE]; // camera chunk in the center
	Block					*closestBlock;
	std::list<Chunk *>		chunksRemoval;

	/* Thread pool */
	int						pool_size;
	bool					pool_state;
	pthread_cond_t *		task_cond;
	volatile bool *			is_task_locked;
	pthread_mutex_t *		task_mutex;
	pthread_t *				threads;
	std::deque<Chunk *> *	task_queue; // one different pool per thread

	Core(void);
	~Core(void);

	uint32_t				getConcurrentThreads();
	void					processChunkGeneration(Chunk *c);
	void *					executeThread(int const &id);
	int						startThreads(void);
	int						stopThreads(void);
	void					addTask(Chunk *c, int const &id);


	/* generation */
	void					createTree(Chunk *c, int const &depth, float x, float y, float z);
	void					generateBlock3d(Chunk *c, float const &x, float const &y, float const &z, int const &depth, int const &ycap);
	void					generateBlock(Chunk *c, float const &x, float const &y, float const &z, int const &depth);
	void					generateChunkMesh(Chunk *chunk, int const &depth) const;
	void					generateChunkGLMesh(Chunk *chunk);
	Block					*getClosestBlock(void) const;

	/* core */
	int						init(void);
	void					updateLeftClick(void);
	void					updateChunks(void);
	void					update(void);
	void					render(void);
	void					loop(void);
	void					clearChunksRemoval(void);
	bool					chunkInTaskPool(Chunk const *chunk) const;

	/* Camera */

	/* textures */
	void					loadTextures(void);
	GLuint					loadTexture(char const *filename);

	/* matrices */ 
	void					buildProjectionMatrix(Mat4<float> &proj, float const &fov,
												float const &near, float const &far);

	void					getLocations(void);

	void					generation(void);
	void					insertChunks(void);
	void					initChunks(void);
	void					createSelectionCube(void);
	void					initNoises(void);

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

#endif
