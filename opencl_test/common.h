#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#define CL_CHECK_ERR(err) \
if (err != CL_SUCCESS) \
{ \
    fprintf(stderr, "OpenCL error with code %d (%s) happened in file %s at line %d.\n", err, get_error_string(err), __FILE__, __LINE__); \
	exit(1); \
}

char *get_error_string(cl_int err);

char *get_platform_info(cl_platform_id platform, cl_platform_info platform_info, char **buffer, int *len);
char *get_device_info(cl_device_id device, cl_device_info device_info, char **buffer, int *len);

void print_platform_names(cl_platform_id *platforms, cl_uint num);
void print_device_names(cl_device_id *devices, cl_uint num);
void print_device_info(cl_device_id device);

int get_platforms(cl_platform_id **platforms);
int get_devices(cl_platform_id platform, cl_device_type device_type, cl_device_id **devices);

cl_platform_id get_platform(const char *platform_string);
cl_device_id get_device(cl_platform_id platform, cl_device_type device_type, const char *device_string);
cl_program get_program_from_file(cl_context context, cl_device_id device, const char *filename);

#endif
