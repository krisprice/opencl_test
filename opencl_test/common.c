#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

#include "common.h"

int get_platforms(cl_platform_id **platforms)
{
	cl_int err;
	cl_uint num;
	
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

void print_platform_names(cl_platform_id *platforms, cl_uint num)
{
	cl_int err;
	char *name;
	unsigned int i;
	size_t name_length = 0;
	size_t previous_name_length = 256;
	
	printf("Platform names:\n");
	
	name = (char *) malloc((previous_name_length+1) * sizeof(char));
	if (name == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}
	
    for (i = 0; i < num; i++)
    {
		previous_name_length = name_length;
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &name_length);
        CL_CHECK_ERR(err);
		
		if (name_length > previous_name_length)
		{
			name = (char *) realloc(name, (name_length+1) * sizeof(char));
			if (name == NULL)
			{
				fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
				exit(1);
			}
		}
		
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, name_length, name, 0);
        CL_CHECK_ERR(err);

		printf("    [%d] %s\n", i, name);
    }

	free(name);
}

int get_devices(cl_platform_id platform, cl_device_id **devices)
{
	cl_int err;
	cl_uint num;
	
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, 0, &num);
    CL_CHECK_ERR(err);
    
	if (num < 1)
		return 0;
	
	*devices = (cl_device_id *) malloc(num * sizeof(cl_device_id));
	if (*devices == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num, *devices, 0);
    CL_CHECK_ERR(err);
	
	return num;
}

void print_device_names(cl_platform_id *devices, cl_uint num)
{
	cl_int err;
	char *name;
	unsigned int i;
	size_t name_length = 0;
	size_t previous_name_length = 256;
	
	printf("Device names:\n");
	
	name = (char *) malloc((previous_name_length+1) * sizeof(char));
	if (name == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	for (i = 0; i < num; i++)
    {
		previous_name_length = name_length;
		err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, 0, &name_length);
        CL_CHECK_ERR(err);
		
		if (name_length > previous_name_length)
		{
			name = (char *) realloc(name, (name_length+1) * sizeof(char));
			if (name == NULL)
			{
				fprintf(stderr, "Failed to allocate memory in %s at line %d\n", __FILE__, __LINE__);
				exit(1);
			}
		}
		
		err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, name_length, name, 0);
        CL_CHECK_ERR(err);

		printf("    [%d] %s\n", i, name);
    }
	
	free(name);
}

cl_platform_id get_platform(const char *platform_string)
{
	cl_int err;
	cl_uint num_of_platforms = 0;
	cl_platform_id *platforms;
	cl_platform_id platform;
	char *platform_name;
	unsigned int i;
	size_t platform_name_length = 0;
	size_t previous_platform_name_length = 256;
	cl_uint selected_platform_index;

	/* Get number of available platforms. */
	err = clGetPlatformIDs(0, 0, &num_of_platforms);
    CL_CHECK_ERR(err);
    
	printf("Number of available platforms: %d\n", num_of_platforms);
	
	if (num_of_platforms < 1)
		return NULL;
	
	/* Get all platforms. */
	platforms = (cl_platform_id *) malloc(num_of_platforms * sizeof(cl_platform_id));
	if (platforms == NULL)
		exit(1);
	
	err = clGetPlatformIDs(num_of_platforms, platforms, 0);
    CL_CHECK_ERR(err);

	/* List all platforms. */
	printf("Platform names:\n");
	
	platform_name = (char *) malloc((previous_platform_name_length+1) * sizeof(char));
	if (platform_name == NULL)
		exit(1);

	selected_platform_index = num_of_platforms;

    for (i = 0; i < num_of_platforms; i++)
    {
		previous_platform_name_length = platform_name_length;
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &platform_name_length);
        CL_CHECK_ERR(err);

		if (platform_name_length > previous_platform_name_length)
		{
			platform_name = (char *) realloc(platform_name, (platform_name_length+1) * sizeof(char));
			if (platform_name == NULL)
				exit(1);
		}
		
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_length, platform_name, 0);
        CL_CHECK_ERR(err);

		printf("    [%d] %s (%d)", i, platform_name, platforms[i]);

		if(strstr(platform_name, platform_string) && selected_platform_index == num_of_platforms)
        {
            printf(" [Selected]");
            selected_platform_index = i;
        }

        printf("\n");
    }
	free(platform_name);

	platform = platforms[selected_platform_index];
	free(platforms);

    if (selected_platform_index == num_of_platforms)
    {
		fprintf(stderr, "There is no found platform with name containing %s\n", platform_string);
        return NULL;
    }
	
	return platform;
}

/* Get the first device that matches a string.
 */
cl_device_id get_device(cl_platform_id platform, cl_device_type device_type, const char *device_string)
{
	cl_int err;
	cl_uint num_of_devices = 0;
	cl_device_id *devices;
	cl_device_id device;
	char *device_name;
	unsigned int i;
	size_t device_name_length = 0;
	size_t previous_device_name_length = 256;
	cl_uint selected_device_index;

	/* Get number of available devices. */
	err = clGetDeviceIDs(platform, device_type, 0, 0, &num_of_devices);
    CL_CHECK_ERR(err);
    
	printf("Number of available devices: %d\n", num_of_devices);
	
	if (num_of_devices < 1)
		return NULL;
	
	/* Get all devices. */
	devices = (cl_device_id *) malloc(num_of_devices * sizeof(cl_device_id));
	if (devices == NULL)
		exit(1);

	err = clGetDeviceIDs(platform, device_type, num_of_devices, devices, 0);
    CL_CHECK_ERR(err);
	
	/* List all devices. */
	printf("Device names:\n");
	
	device_name = (char *) malloc((previous_device_name_length+1) * sizeof(char));
	if (device_name == NULL)
		exit(1);

	selected_device_index = num_of_devices;

    for (i = 0; i < num_of_devices; i++)
    {
		previous_device_name_length = device_name_length;
		err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, 0, &device_name_length);
        CL_CHECK_ERR(err);

		if (device_name_length > previous_device_name_length)
		{
			device_name = (char *) realloc(device_name, (device_name_length+1) * sizeof(char));
			if (device_name == NULL)
				exit(1);
		}
		
		err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, device_name_length, device_name, 0);
        CL_CHECK_ERR(err);

		printf("    [%d] %s (%d)", i, device_name, devices[i]);

		if(strstr(device_name, device_string) && selected_device_index == num_of_devices)
        {
            printf(" [Selected]");
            selected_device_index = i;
        }

        printf("\n");
    }
	free(device_name);
	
	device = devices[selected_device_index];
	free(devices);

    if (selected_device_index == num_of_devices)
    {
		fprintf(stderr, "There is no found device with name containing %s\n", device_string);
        return NULL;
    }
	
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