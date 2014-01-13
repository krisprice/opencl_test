#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

#include "common.h"

char *get_error_string(cl_int err)
{
	switch (err)
	{
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
		default: return "Unknown error code";
	}
}

char *get_platform_info(cl_platform_id platform, cl_platform_info platform_info, char **buffer, int *len)
{
	cl_int err;
	int required_len = 256;
	
	do
	{
		if (*len < required_len)
		{
			*buffer = (char *) realloc(*buffer, required_len * sizeof(char));
			if (buffer == NULL)
			{
				fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
				return NULL;
			}

			*len = required_len;
		}

		err = clGetPlatformInfo(platform, platform_info, *len, *buffer, &required_len);
		CL_CHECK_ERR(err);
	} while (*len < required_len);

	return *buffer;
}

char *get_device_info(cl_device_id device, cl_device_info device_info, char **buffer, int *len)
{
	cl_int err;
	int required_len = 256;

	do
	{
		if (*len < required_len)
		{
			*buffer = (char *)realloc(*buffer, required_len * sizeof(char));
			if (buffer == NULL)
			{
				fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
				return NULL;
			}

			*len = required_len;
		}

		err = clGetDeviceInfo(device, device_info, *len, *buffer, &required_len);
		CL_CHECK_ERR(err);
	} while (*len < required_len);

	return *buffer;
}

void print_platform_names(cl_platform_id *platforms, cl_uint num)
{
	char *name;
	int len;
	cl_uint i;

	printf("Platform names:\n", num);

	name = NULL;
	len = 0;
	for (i = 0; i < num; i++)
	{
		get_platform_info(platforms[i], CL_PLATFORM_NAME, &name, &len);
		printf("    [%d] %s\n", i, name);
	}

	if (name != NULL)
		free(name);
}

void print_device_names(cl_device_id *devices, cl_uint num)
{
	char *name;
	int len;
	cl_uint i;

	printf("Device names:\n", num);

	name = NULL;
	len = 0;
	for (i = 0; i < num; i++)
	{
		get_device_info(devices[i], CL_DEVICE_NAME, &name, &len);
		printf("    [%d] %s\n", i, name);
	}

	if (name != NULL)
		free(name);
}

#define CL_PRINT_DEVICE_INFO_STRING(info) err = clGetDeviceInfo(device, info, sizeof(tmp), tmp, 0); CL_CHECK_ERR(err); printf(#info " = %s\n", s); 
#define CL_PRINT_DEVICE_INFO_UINT(info) err = clGetDeviceInfo(device, info, sizeof(tmp), tmp, 0); CL_CHECK_ERR(err); printf(#info " = %u\n", *tmp); 
#define CL_PRINT_DEVICE_INFO_ULONG(info) err = clGetDeviceInfo(device, info, sizeof(tmp), tmp, 0); CL_CHECK_ERR(err); printf(#info " = %llu\n", *tmp); 

void print_device_info(cl_device_id device)
{
	cl_int err;
	cl_ulong tmp[256];
	char *s = (char *) tmp;

	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_TYPE); // cl_device_type
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_VENDOR_ID);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_COMPUTE_UNITS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_WORK_GROUP_SIZE); // size_t
	//CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_WORK_ITEM_SIZES); // size_t[]
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_CLOCK_FREQUENCY);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_ADDRESS_BITS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_READ_IMAGE_ARGS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
	CL_PRINT_DEVICE_INFO_ULONG(CL_DEVICE_MAX_MEM_ALLOC_SIZE);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE2D_MAX_WIDTH); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE2D_MAX_HEIGHT); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE3D_MAX_WIDTH); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE3D_MAX_HEIGHT); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE3D_MAX_DEPTH); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_IMAGE_SUPPORT); // cl_bool
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_PARAMETER_SIZE); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_SAMPLERS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MEM_BASE_ADDR_ALIGN);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_SINGLE_FP_CONFIG); // cl_device_fp_config
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE); // cl_device_mem_cache_type
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
	CL_PRINT_DEVICE_INFO_ULONG(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
	CL_PRINT_DEVICE_INFO_ULONG(CL_DEVICE_GLOBAL_MEM_SIZE);
	CL_PRINT_DEVICE_INFO_ULONG(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_MAX_CONSTANT_ARGS);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_LOCAL_MEM_TYPE); // cl_device_local_mem_type
	CL_PRINT_DEVICE_INFO_ULONG(CL_DEVICE_LOCAL_MEM_SIZE);
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_ERROR_CORRECTION_SUPPORT); // cl_bool
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PROFILING_TIMER_RESOLUTION); // size_t
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_ENDIAN_LITTLE); // cl_bool
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_AVAILABLE); // cl_bool
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_COMPILER_AVAILABLE); // cl_bool
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_EXECUTION_CAPABILITIES); // cl_device_exec_capabilities
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_QUEUE_PROPERTIES); // cl_command_queue_properties
	CL_PRINT_DEVICE_INFO_STRING(CL_DEVICE_NAME);
	CL_PRINT_DEVICE_INFO_STRING(CL_DEVICE_VENDOR);
	CL_PRINT_DEVICE_INFO_STRING(CL_DRIVER_VERSION);
	CL_PRINT_DEVICE_INFO_STRING(CL_DEVICE_PROFILE);
	CL_PRINT_DEVICE_INFO_STRING(CL_DEVICE_VERSION);
	//CL_PRINT_DEVICE_INFO_STRING(CL_DEVICE_EXTENSIONS); // Gets CL_INVALID_VALUE on Intel OpenCL
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_PLATFORM); // cl_platform_id
	CL_PRINT_DEVICE_INFO_UINT(CL_DEVICE_DOUBLE_FP_CONFIG); // cl_device_fp_config
}

int get_platforms(cl_platform_id **platforms)
{
	cl_int err;
	cl_uint num;
	
	*platforms = NULL;

	err = clGetPlatformIDs(0, 0, &num);
    CL_CHECK_ERR(err);
    
	if (num < 1)
		return 0;
	
	*platforms = (cl_platform_id *) malloc(num * sizeof(cl_platform_id));
	if (*platforms == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}
	
	err = clGetPlatformIDs(num, *platforms, 0);
    CL_CHECK_ERR(err);

	return num;
}

int get_devices(cl_platform_id platform, cl_device_type device_type, cl_device_id **devices)
{
	cl_int err;
	cl_uint num;

	*devices = NULL;

	err = clGetDeviceIDs(platform, device_type, 0, 0, &num);
    CL_CHECK_ERR(err);
    
	if (num < 1)
		return 0;
	
	*devices = (cl_device_id *) malloc(num * sizeof(cl_device_id));
	if (*devices == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	err = clGetDeviceIDs(platform, device_type, num, *devices, 0);
    CL_CHECK_ERR(err);
	
	return num;
}

/* Get first platform that matches a string.
 */
cl_platform_id get_platform(const char *platform_string)
{
	cl_platform_id *platforms = NULL;
	cl_platform_id platform;
	int i;
	int num;
	char *name;
	int len;

	num = get_platforms(&platforms);
	if (num < 1)
		return NULL;
	
	platform = NULL;
	name = NULL;
	len = 0;
    for (i = 0; i < num; i++)
    {	
		get_platform_info(platforms[i], CL_PLATFORM_NAME, &name, &len);

		if (strstr(name, platform_string))
        {
			platform = platforms[i];
			break;
        }
    }

	if (name != NULL)
		free(name);
	
	if (platforms != NULL)
		free(platforms);
	
	return platform;
}

/* Get the first device that matches the given device type and string.
 */
cl_device_id get_device(cl_platform_id platform, cl_device_type device_type, const char *device_string)
{
	cl_device_id *devices = NULL;
	cl_device_id device;
	char *name;
	int i;
	int len;
	int num;

	num = get_devices(platform, device_type, &devices);
	if (num < 1)
		return NULL;
	
	device = NULL;
	name = NULL;
	len = 0;
    for (i = 0; i < num; i++)
    {
		get_device_info(devices[i], CL_DEVICE_NAME, &name, &len);
	
		if (strstr(name, device_string))
        {
			device = devices[i];
			break;
        }
	}

	if (name != NULL)
		free(name);
	
	if (devices != NULL)
		free(devices);
	
	return device;
}

cl_program get_program_from_file(cl_context context, cl_device_id device, const char *filename)
{
	FILE *fp;
	int size;
	char *buffer;
	cl_int err;
	cl_program program;
	char buf[100000];
	
	/* Read file into buffer. */
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open file: %s\n", filename);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	buffer = (char *) malloc((size+1) * sizeof(char));
	buffer[size] = '\0';
	fread(buffer, sizeof(char), size, fp);
	fclose(fp);

	/* Create program. */
	program = clCreateProgramWithSource(context, 1, &buffer, NULL, &err);
	CL_CHECK_ERR(err);

	/* Build program. */
	if (clBuildProgram(program, 1, &device, "", NULL, NULL) != CL_SUCCESS)
	{
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 100000, buf, NULL);
		fprintf(stderr, "CL Compilation failed:\n%s", buffer);
		exit(1);
	}
	free(buffer);

	err = clUnloadCompiler();
	CL_CHECK_ERR(err);

	return program;
}
