
#ifndef CORE_HPP
# define CORE_HPP

# include "Camera.hpp"
# include "Shaders.hpp"
# include "Bmp.hpp"
# include "Link.hpp"

class Link;

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

	GLuint					voxelVao;
	GLuint					voxelVbo[2];

	/* Camera */
	Camera					camera;

	/* mouse movement */
	double					lastMx;
	double					lastMy;

	/* Textures */
	int						texMax;
	GLuint					*tex;

	float					multiplier;
	std::ostringstream		oss_ticks;

	Link					*octree;

	Core(void);
	~Core(void);

	/* core */
	int						init(void);
	void					update(void);
	void					render(void);
	void					loop(void);

	/* Camera */

	/* textures */
	void					loadTextures(void);
	GLuint					loadTexture(char const *filename);

	/* matrices */ 
	void					setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
										Vec3<float> const &right, Vec3<float> const &up);
	void					buildProjectionMatrix(Mat4<float> &proj, float const &fov,
												float const &near, float const &far);

	void					getLocations(void);

	/* tests */
	void					initVoxel(void);

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

#endif
