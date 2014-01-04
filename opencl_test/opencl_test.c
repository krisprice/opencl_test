#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>
#include <Windows.h>

#include "common.h"

#define GLOBAL_SIZE 1024
#define LOCAL_SIZE 16

void run_get_ids(cl_context context, cl_command_queue queue, cl_program program)
{
	cl_int err;
	cl_kernel kernel;
	cl_event ev[5];
	cl_mem global_ids_buf;
	cl_mem group_ids_buf;
	cl_mem local_ids_buf;
	int global_ids[GLOBAL_SIZE];
	int group_ids[GLOBAL_SIZE];
	int local_ids[GLOBAL_SIZE];
	size_t global_size = GLOBAL_SIZE;
	size_t local_size = LOCAL_SIZE;
	int i;

	/* Create buffers. */
	global_ids_buf = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, GLOBAL_SIZE*sizeof(int), global_ids, &err);
	group_ids_buf = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, GLOBAL_SIZE*sizeof(int), group_ids, &err);
	local_ids_buf = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, GLOBAL_SIZE*sizeof(int), local_ids, &err);
	CL_CHECK_ERR(err);
	
	/* Create kernel. */
	kernel = clCreateKernel(program, "get_ids", &err);
	CL_CHECK_ERR(err);
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &global_ids_buf);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &group_ids_buf);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &local_ids_buf);
	CL_CHECK_ERR(err);

	/* Enqueue kernel. */
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &ev[0]); 
	CL_CHECK_ERR(err);
	
	/* Copy the output. */
	err = clEnqueueReadBuffer(queue, global_ids_buf, CL_FALSE, 0, GLOBAL_SIZE*sizeof(int), global_ids, 0, NULL, &ev[1]);
	err = clEnqueueReadBuffer(queue, group_ids_buf, CL_FALSE, 0, GLOBAL_SIZE*sizeof(int), group_ids, 0, NULL, &ev[2]);
	err = clEnqueueReadBuffer(queue, local_ids_buf, CL_FALSE, 0, GLOBAL_SIZE*sizeof(int), local_ids, 0, NULL, &ev[3]);
	CL_CHECK_ERR(err);

	err = clWaitForEvents(4, ev);
	CL_CHECK_ERR(err);

	err = clReleaseEvent(ev[0]);
	err = clReleaseEvent(ev[1]);
	err = clReleaseEvent(ev[2]);
	err = clReleaseEvent(ev[3]);
	CL_CHECK_ERR(err);

	clFinish(queue); // should be unnecessary since we're waiting for events above

	/* Print result. */
	for (i = 0; i < GLOBAL_SIZE; i++)
		printf("global_id = %d group_id = %d local_id = %d\n", global_ids[i], group_ids[i], local_ids[i]);

	/* Clean up. */
	err = clReleaseMemObject(global_ids_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(group_ids_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(local_ids_buf); CL_CHECK_ERR(err);
	err = clReleaseKernel(kernel); CL_CHECK_ERR(err);
}

void run_sum_numbers(cl_context context, cl_command_queue queue, cl_program program)
{
	cl_int err;
	cl_kernel kernel;
	cl_event ev[5];
	cl_mem numbers_buf;
	cl_mem sums_buf;
	int *numbers;
	int *sums;
	int total;
	size_t global_size = GLOBAL_SIZE;
	size_t local_size = LOCAL_SIZE;
	size_t num_groups = (global_size / local_size);
	int i;

	/* Create buffers. */
	numbers = (int *) malloc(global_size * global_size * sizeof(int));
	sums = (int *) malloc(num_groups * sizeof(int));
	
	if (numbers == NULL || sums == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in file %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	for (i = 0; i < global_size * global_size; i++)
		numbers[i] = 1;

	for (i = 0; i < global_size; i++)
		sums[i] = 0;

	numbers_buf = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, global_size * global_size * sizeof(float), numbers, &err);
	sums_buf = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), sums, &err);
	CL_CHECK_ERR(err);

	/* Create kernel. */
	kernel = clCreateKernel(program, "sum_numbers", &err);
	CL_CHECK_ERR(err);
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &numbers_buf);
	err = clSetKernelArg(kernel, 1, local_size * sizeof(float), NULL);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &sums_buf);
	CL_CHECK_ERR(err);
	
	/* Enqueue kernel. */
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &ev[0]); 
	CL_CHECK_ERR(err);
	
	/* Copy the output. */
	err = clEnqueueReadBuffer(queue, sums_buf, CL_FALSE, 0, num_groups * sizeof(float), sums, 0, NULL, &ev[1]);
	CL_CHECK_ERR(err);

	err = clWaitForEvents(2, ev);
	CL_CHECK_ERR(err);

	err = clReleaseEvent(ev[0]);
	err = clReleaseEvent(ev[1]);
	CL_CHECK_ERR(err);
	
	clFinish(queue); // should be unnecessary since we're waiting for events above

	/* Print result. */
	total = 0;

	for (i = 0; i < num_groups; i++)
		total += sums[i];
	
	printf("OpenCL sum = %d\n", total);
	total = 1 * global_size * global_size;
	printf("Normal sum = %d\n", total);

	err = clReleaseMemObject(numbers_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(sums_buf); CL_CHECK_ERR(err);
	err = clReleaseKernel(kernel); CL_CHECK_ERR(err);
}

void run_matrix_multiply(cl_context context, cl_command_queue queue, cl_program program)
{
	cl_int err;
	cl_kernel kernel;
	cl_event ev[5];
	cl_mem aa_buf;
	cl_mem b_buf;
	cl_mem c_buf;
	float *aa;
	float *b;
	float *c;
	size_t global_size = GLOBAL_SIZE;
	size_t local_size = LOCAL_SIZE;
	int i, j;
	unsigned int n;
	
	/* Create buffers. */
	n = GLOBAL_SIZE;
	
	aa = (float *) malloc(n * n * sizeof(float));
	b = (float *) malloc(n * sizeof(float));
	c = (float *) malloc(n * sizeof(float));

	if (aa == NULL || b == NULL || c == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in file %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	for (i = 0; i < GLOBAL_SIZE; i++)
	{
		for(j = 0; j < GLOBAL_SIZE; j++)
			aa[i*GLOBAL_SIZE+j] = (float) 1.1 * i * j;

		b[i] = (float) 2.2 * i;
		c[i] = (float) 0.0;
	}

	aa_buf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, GLOBAL_SIZE * GLOBAL_SIZE * sizeof(float), aa, &err);
	b_buf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, GLOBAL_SIZE * sizeof(float), b, &err);
	c_buf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, GLOBAL_SIZE * sizeof(float), c, &err);
	CL_CHECK_ERR(err);
	
	/* Create kernel. */
	kernel = clCreateKernel(program, "matrix_multiply", &err);
	CL_CHECK_ERR(err);
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_uint), &n);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &aa_buf);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &b_buf);
	err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &c_buf);
	CL_CHECK_ERR(err);
	
	/* Enqueue kernel. */
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &ev[0]); 
	CL_CHECK_ERR(err);
	
	/* Copy the output. */
	err = clEnqueueReadBuffer(queue, c_buf, CL_FALSE, 0, GLOBAL_SIZE * sizeof(float), c, 0, NULL, &ev[1]);
	CL_CHECK_ERR(err);

	err = clWaitForEvents(2, ev);
	CL_CHECK_ERR(err);

	err = clReleaseEvent(ev[0]);
	err = clReleaseEvent(ev[1]);
	CL_CHECK_ERR(err);
	
	clFinish(queue); // should be unnecessary since we're waiting for events above
	
	/* Print result. */
	
	for(i = 0; i < n; i++)
		printf("c[%d] %f\n", i, c[i]);

	err = clReleaseMemObject(aa_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(b_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(c_buf); CL_CHECK_ERR(err);
	err = clReleaseKernel(kernel); CL_CHECK_ERR(err);
	
	free(aa);
	free(b);
	free(c);
}

#define KEY_LEN 100

#define l3_rotate(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define l3_mix(a, b, c) \
{ \
	a -= c; a ^= l3_rotate(c, 4); c += b; \
	b -= a; b ^= l3_rotate(a, 6); a += c; \
	c -= b; c ^= l3_rotate(b, 8); b += a; \
	a -= c; a ^= l3_rotate(c,16); c += b; \
	b -= a; b ^= l3_rotate(a,19); a += c; \
	c -= b; c ^= l3_rotate(b, 4); b += a; \
}

#define l3_final(a,b,c) \
{ \
  c ^= b; c -= l3_rotate(b,14); \
  a ^= c; a -= l3_rotate(c,11); \
  b ^= a; b -= l3_rotate(a,25); \
  c ^= b; c -= l3_rotate(b,16); \
  a ^= c; a -= l3_rotate(c,4);  \
  b ^= a; b -= l3_rotate(a,14); \
  c ^= b; c -= l3_rotate(b,24); \
}

unsigned int lookup3(char *key, int len, int seed)
{
	unsigned int a, b, c;
	char *k8;
	unsigned int *k;
	
	a = b = c = 0xdeadbeef + (((unsigned int) len) << 2) + seed;
	k = (unsigned int *) key;

	while (len > 12)
	{
		a += k[0];
		b += k[1];
		c += k[2];
		l3_mix(a,b,c);
		len -= 12;
		k += 3;
	}

	k8 = (char *) k;

    switch(len)
    {
		case 12:
			c+=k[4]+(((unsigned int)k[5])<<16);
			b+=k[2]+(((unsigned int)k[3])<<16);
			a+=k[0]+(((unsigned int)k[1])<<16);
		break;
		case 11:
			c+=((unsigned int)k8[10])<<16; // fall through
		case 10:
			c+=k[4];
			b+=k[2]+(((unsigned int)k[3])<<16);
			a+=k[0]+(((unsigned int)k[1])<<16);
		break;
		case 9:
			c+=k8[8]; // fall through
		case 8:
			b+=k[2]+(((unsigned int)k[3])<<16);
			a+=k[0]+(((unsigned int)k[1])<<16);
		break;
		case 7:
			b+=((unsigned int)k8[6])<<16; // fall through
		case 6:
			b+=k[2];
			a+=k[0]+(((unsigned int)k[1])<<16);
		break;
		case 5:
			b+=k8[4]; // fall through
		case 4:
			a+=k[0]+(((unsigned int)k[1])<<16);
		break;
		case 3:
			a+=((unsigned int)k8[2])<<16; // fall through
		case 2:
			a+=k[0];
		break;
		case 1:
			a+=k8[0];
			break;
		case 0:
			return c; // zero length requires no mixing
    }

	l3_final(a, b, c);
	return c;
}

void run_hash_test(cl_context context, cl_command_queue queue, cl_program program)
{
	cl_int err;
	cl_kernel kernel;
	cl_event ev[5];
	cl_mem keys_buf;
	cl_mem hashes_buf;
	char *keys;
	unsigned int len = KEY_LEN;
	unsigned int seed = 0;
	unsigned int *hashes;
	size_t global_size = GLOBAL_SIZE;
	size_t local_size = LOCAL_SIZE;
	char *charset = "abcdefghijklmnopqrstuvwxyz";
	int i, l;
	
	/* Create buffers. */
	keys = (char *) malloc(global_size * len * sizeof(char));
	hashes = (unsigned int *) malloc(global_size * sizeof(int));

	if (keys == NULL || hashes == NULL)
	{
		fprintf(stderr, "Failed to allocate memory in file %s at line %d\n", __FILE__, __LINE__);
		exit(1);
	}
	
	l = strlen(charset);
	for (i = 0; (i + l) < (global_size * len); i += l)
		strncpy(&keys[i], charset, l);
	strncpy(&keys[i], charset, (global_size * len) % l);
	
	keys_buf = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, global_size * KEY_LEN * sizeof(char), keys, &err);
	hashes_buf = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, global_size * sizeof(unsigned int), hashes, &err);
	CL_CHECK_ERR(err);
	
	/* Create kernel. */
	kernel = clCreateKernel(program, "lookup3_hash_keys", &err);
	CL_CHECK_ERR(err);
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &keys_buf);
	err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &len);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &seed);
	err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &hashes_buf);
	CL_CHECK_ERR(err);
	
	/* Enqueue kernel. */
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &ev[0]); 
	CL_CHECK_ERR(err);
	
	/* Copy the output. */
	err = clEnqueueReadBuffer(queue, hashes_buf, CL_FALSE, 0, global_size * sizeof(unsigned int), hashes, 0, NULL, &ev[1]);
	CL_CHECK_ERR(err);

	err = clWaitForEvents(2, ev);
	CL_CHECK_ERR(err);

	err = clReleaseEvent(ev[0]);
	err = clReleaseEvent(ev[1]);
	CL_CHECK_ERR(err);
	
	clFinish(queue); // should be unnecessary since we're waiting for events above
	
	/* Print result. */
	printf("run_hash_test():\n");

	for (i = 0; i < global_size; i++)
	{
		l = lookup3(&keys[i*len], len, seed);
		printf("%d: %d %d\n", i, hashes[i], l);
	}

	/* Clean up. */
	err = clReleaseMemObject(keys_buf); CL_CHECK_ERR(err);
	err = clReleaseMemObject(hashes_buf); CL_CHECK_ERR(err);
	err = clReleaseKernel(kernel); CL_CHECK_ERR(err);
	
	free(keys);
	free(hashes);
}

#define NLOOPS 500

void run_minp_test(cl_context context, cl_command_queue queue, cl_program program)
{
	cl_int err;
	cl_kernel minp;
	cl_kernel reduce;
	cl_event ev;
	unsigned int num_src_items = 4096*4096;
	int i;
	INT64 c0, c1, freq;
	double elapsed;
	int nloops = NLOOPS;
	int dev, nw;
	cl_uint ws = 64;
	time_t ltime;
	cl_uint *src_ptr;
	cl_uint a, b, min;
	cl_mem src_buf, dst_buf, dbg_buf;
	cl_uint *dst_ptr, *dbg_ptr;
	cl_uint compute_units;
	size_t global_work_size, local_work_size, num_groups;
	cl_device_id device;
	cl_device_type device_type;

	time(&ltime);
	src_ptr = (cl_uint *) malloc(num_src_items * sizeof(cl_uint));
	a = (cl_uint) ltime,
	b = (cl_uint) ltime;
	min = (cl_uint) -1;

	for (i = 0; i < num_src_items; i++)
	{
		src_ptr[i] = (cl_uint) (b = ( a * (b & 65535)) + (b >> 16));
		min	= src_ptr[i] < min ? src_ptr[i] : min;
	}
	
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 1, &device, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &compute_units, NULL);
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_uint), &device_type, NULL);

	if (device_type == CL_DEVICE_TYPE_CPU)
	{
		global_work_size = compute_units * 1; // 1 thread per core
		local_work_size = 1;
	}
	else
	{
		global_work_size = compute_units * 7 * ws; // 7 wavefronts per SIMD
		while ((num_src_items / 4) % global_work_size != 0)
			global_work_size += ws;
		local_work_size = ws;
	}
		
	num_groups = global_work_size / local_work_size;

	minp = clCreateKernel(program, "minp", NULL);
	reduce = clCreateKernel(program, "reduce", NULL);

	src_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, num_src_items * sizeof(cl_uint), src_ptr, NULL);
	dst_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, num_groups * sizeof(cl_uint), NULL, NULL); 
	dbg_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, global_work_size * sizeof(cl_uint), NULL, NULL);
		
	clSetKernelArg(minp, 0, sizeof(void *), (void*) &src_buf);
	clSetKernelArg(minp, 1, sizeof(void *), (void*) &dst_buf);
	clSetKernelArg(minp, 2, sizeof(cl_uint), (void*) NULL);
	clSetKernelArg(minp, 3, sizeof(void *), (void*) &dbg_buf);
	clSetKernelArg(minp, 4, sizeof(num_src_items), (void*) &num_src_items);
	clSetKernelArg(minp, 5, sizeof(dev), (void*) &dev);
	clSetKernelArg(reduce, 0, sizeof(void *), (void*) &src_buf);
	clSetKernelArg(reduce, 1, sizeof(void *), (void*) &dst_buf);
		
	QueryPerformanceCounter(&c0);

	while(nloops--) 
	{
		clEnqueueNDRangeKernel( queue, minp, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &ev);
		clEnqueueNDRangeKernel( queue, reduce, 1, NULL, &num_groups, NULL, 1, &ev, NULL);
	}

	clFinish(queue);
	
	QueryPerformanceCounter(&c1);
	QueryPerformanceFrequency(&freq);
	elapsed = (c1 - c0) / freq;

	printf("B/W %.2f GB/sec, ", ((float) num_src_items * sizeof(cl_uint) * NLOOPS) / elapsed / 1e9);

	dst_ptr = (cl_uint *) clEnqueueMapBuffer(queue, dst_buf, CL_TRUE, CL_MAP_READ, 0,  num_groups * sizeof(cl_uint), 0, NULL, NULL, NULL);
	dbg_ptr = (cl_uint *) clEnqueueMapBuffer(queue, dbg_buf, CL_TRUE, CL_MAP_READ, 0,  global_work_size * sizeof(cl_uint), 0, NULL, NULL, NULL);
		
	printf("%d groups, %d threads, count %d, stride %d\n", dbg_ptr[0], dbg_ptr[1], dbg_ptr[2], dbg_ptr[3]);
	if (dst_ptr[0] == min)
		printf("result correct\n");
	else
		printf("result incorrect\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	cl_int err;
	cl_platform_id *platforms = NULL;
	cl_int num_platforms;
	cl_device_id *devices = NULL;
	cl_int num_devices;
	
	cl_context context;
	cl_command_queue queue;
	cl_program program;

	int i, j;

	/* Iterate over the platforms and devices running the kernels.
	 */
	num_platforms = get_platforms(&platforms);
	
	if (num_platforms > 0)
		print_platform_names(platforms, num_platforms);

	for (i = 0; i < num_platforms; i++)
	{
		num_devices = get_devices(platforms[i], &devices);

		if (num_devices > 0)
			print_device_names(devices, num_devices);

		for (j = 0; j < num_devices; j++)
		{
			/* Get context. */
			context = clCreateContext(NULL, 1, &devices[j], NULL, NULL, &err); // TODO: should bother to specify platform in properties?
			CL_CHECK_ERR(err);
	
			/* Create a command queue. */
			queue = clCreateCommandQueue(context, devices[j], 0, &err);
			CL_CHECK_ERR(err);

			/* Build program from source file. */
			program = get_program_from_file(context, devices[j], "test.cl");

			/* Run some kernels. */
			//run_get_ids(context, queue, program);
			//run_sum_numbers(context, queue, program);
			run_matrix_multiply(context, queue, program);
			//run_hash_test(context, queue, program);
			//run_minp_test();

			/* Clean up. */
			err = clReleaseProgram(program); CL_CHECK_ERR(err);
			err = clReleaseCommandQueue(queue); CL_CHECK_ERR(err);
			err = clReleaseContext(context); CL_CHECK_ERR(err);	
		}
	}

	free(platforms);
	free(devices);

	return 0;
}
