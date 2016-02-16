
#ifndef CLWRAPPER_HPP
# define CLWRAPPER_HPP

# ifdef __APPLE__
#  include <OpenCL/opencl.h>
#  include <OpenGL/CGLTypes.h>
#  include <OpenGL/CGLCurrent.h>
# else
#  include <CL/cl.h>
#  include <CL/cl_gl.h>
#  include <GL/glx.h>
# endif

# include <iostream>
# include "Utils.hpp"

# define N_PROGRAM			1

class CLWrapper
{
public:
	CLWrapper(void);
	~CLWrapper(void);

	cl_int					init(void);
	cl_int					createCommandQueue(cl_command_queue *cq);

private:
	size_t					local[N_PROGRAM];
	// size_t					global;
	cl_uint					num_entries;
	cl_platform_id			platformId;
	cl_uint					num_platforms;
	cl_device_id			deviceId;
	cl_context				context;
	// cl_command_queue		clCommands;
	cl_program				programs[N_PROGRAM];
	cl_kernel				kernels[N_PROGRAM];
};

#endif