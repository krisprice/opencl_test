__kernel void get_ids(
	__global int *global_ids,
	__global int *group_ids,
	__global int *local_ids)
{
	int id = get_global_id(0);
	global_ids[id] = id;
	group_ids[id] = get_group_id(0);
	local_ids[id] = get_local_id(0);
}

__kernel void sum_numbers(
	__global int *numbers,
	__local int *local_sums,
	__global int *group_sums)
{
	int global_id = get_global_id(0);
	int local_id = get_local_id(0);
	int global_size = get_global_size(0);
	int i;
	int sum = 0;

	for (i = 0; i < global_size; i++)
		sum += numbers[(global_id*global_size)+i];
	
	local_sums[local_id] = sum;

	barrier(CLK_LOCAL_MEM_FENCE);

	if (get_local_id(0) == 0)
	{
		sum = 0;
		for (i = 0; i < get_local_size(0); i++)
			sum += local_sums[i];
		group_sums[get_group_id(0)] = sum;
	}
}

__kernel void matrix_multiply(
	uint n,
	__global float *aa,
	__global float *b,
	__global float *c)
{
	int i = get_global_id(0);
	int j;
	float tmp = 0.0f;
	for (j = 0; j < n; j++)
		tmp += aa[i*n+j] * b[j];
	c[i] = aa[i*n+i];
}

/* lookup3 */

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

uint lookup3(char *key, int len, int seed)
{
	uint a, b, c;
	char *k8;
	uint *k;
	
	a = b = c = 0xdeadbeef + (((uint) len) << 2) + seed;
	k = (uint *) key;

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
			c+=k[4]+(((uint)k[5])<<16);
			b+=k[2]+(((uint)k[3])<<16);
			a+=k[0]+(((uint)k[1])<<16);
		break;
		case 11:
			c+=((uint)k8[10])<<16; // fall through
		case 10:
			c+=k[4];
			b+=k[2]+(((uint)k[3])<<16);
			a+=k[0]+(((uint)k[1])<<16);
		break;
		case 9:
			c+=k8[8]; // fall through
		case 8:
			b+=k[2]+(((uint)k[3])<<16);
			a+=k[0]+(((uint)k[1])<<16);
		break;
		case 7:
			b+=((uint)k8[6])<<16; // fall through
		case 6:
			b+=k[2];
			a+=k[0]+(((uint)k[1])<<16);
		break;
		case 5:
			b+=k8[4]; // fall through
		case 4:
			a+=k[0]+(((uint)k[1])<<16);
		break;
		case 3:
			a+=((uint)k8[2])<<16; // fall through
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

__kernel void lookup3_hash_keys(
	__global char *keys,
	int len,
	int seed,
	__global uint *hashes)
{
	uint gid = get_global_id(0);
	hashes[gid] = lookup3((uint *) &keys[gid*len], len, seed);
}

/* Parallel min example from AMD APP Programming Guide. */
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable

__kernel void minp(
	__global uint4 *src,
	__global uint *gmin,
	__local  uint  *lmin,
	__global uint *dbg,
	int nitems,
	uint dev)
{
	uint count = (nitems / 4) / get_global_size(0);
	uint idx = (dev == 0) ? get_global_id(0) * count : get_global_id(0);
	uint stride = (dev == 0) ? 1 : get_global_size(0);
	uint pmin = (uint) -1;
	
	for (int n = 0; n < count; n++, idx += stride)
	{
		pmin = min( pmin, src[idx].x );
		pmin = min( pmin, src[idx].y );
		pmin = min( pmin, src[idx].z );
		pmin = min( pmin, src[idx].w );
	}
	
	if (get_local_id(0) == 0)
		lmin[0] = (uint) -1;

	barrier(CLK_LOCAL_MEM_FENCE);
	
	(void) atom_min(lmin, pmin); 

	barrier(CLK_LOCAL_MEM_FENCE);
	
	if (get_local_id(0) == 0)
		gmin[get_group_id(0)] = lmin[0];
	
	if (get_global_id(0) == 0)
	{
		dbg[0] = get_num_groups(0);
		dbg[1] = get_global_size(0);
		dbg[2] = count;
		dbg[3] = stride;
	}
}

__kernel void reduce(__global uint4 *src, __global uint *gmin)
{
	(void) atom_min(gmin, gmin[get_global_id(0)]);
}
