
#include "CLWrapper.hpp"

CLWrapper::CLWrapper(void)
{
	return ;
}

CLWrapper::~CLWrapper(void)
{
	return ;
}

cl_int
CLWrapper::createCommandQueue(cl_command_queue *cq)
{
	int						err;

	if (!cq)
		return (printError("Error: Failed to create command queue!", EXIT_FAILURE));
	*cq = clCreateCommandQueue(context, deviceId, 0, &err);
	if (!(*cq) || err != CL_SUCCESS)
		return (printError("Error: Failed to create command queue!", EXIT_FAILURE));
	return (CL_SUCCESS);
}

cl_int
CLWrapper::init(void)
{
	int							err;
	int							i;
	size_t						len;
	char						buffer[2048];
	static char const			*options = "-I./inc -I./kernels -Werror -cl-fast-relaxed-math";
	static char const			*programNames[N_PROGRAM] =
	{
		"generateBlock"
	};
	static char const			*kernelFiles[N_PROGRAM] =
	{
		"./kernels/generateBlock.cl"
	};
	std::string					file_content;
	char						*file_string;

	num_entries = 1;
	err = clGetPlatformIDs(num_entries, &platformId, &num_platforms);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to retrieve platform id ! " << err, EXIT_FAILURE));

	err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, 0);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to create a device group ! " << err, EXIT_FAILURE));

	context = clCreateContext(0, 1, &deviceId, 0, 0, &err);
	if (!context || err != CL_SUCCESS)
		return (printError("Error: Failed to create a compute context !", EXIT_FAILURE));

	for (i = 0; i < N_PROGRAM; ++i)
	{
		file_content = getFileContents(kernelFiles[i]);
		file_string = (char *)file_content.c_str();
		programs[i] = clCreateProgramWithSource(context, 1, (char const **)&file_string, 0, &err);
		if (!programs[i] || err != CL_SUCCESS)
		{
			return (printError(std::ostringstream().flush()
								<< "Error Failed to create compute "
								<< programNames[i]
								<< " program !",
								EXIT_FAILURE));
		}
		err = clBuildProgram(programs[i], 0, 0, options, 0, 0);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error: Failed to build program executable ! " << err << std::endl;
			clGetProgramBuildInfo(programs[i], deviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return (EXIT_FAILURE);
		}
		kernels[i] = clCreateKernel(programs[i], programNames[i], &err);
		if (!kernels[i] || err != CL_SUCCESS)
			return (printError("Error: Failed to create compute kernel !", EXIT_FAILURE));
		err = clGetKernelWorkGroupInfo(kernels[i], deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &local[i], NULL);
		if (err != CL_SUCCESS)
			return (printError(std::ostringstream().flush() << "Error: Failed to retrieve kernel work group info! " << err, EXIT_FAILURE));
	}
	return (CL_SUCCESS);
}
