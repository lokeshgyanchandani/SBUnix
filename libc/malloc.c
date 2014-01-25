#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

void *malloc(uint64_t no_bytes)
{
	char *tmp = NULL; 
	uint32_t div = 0;
	uint32_t nu = 0, final = 0;
	char *pt_mal = NULL;
	uint64_t size_left = 0;

	if (!no_bytes)
	{
		return NULL;
	}

	if (pt_mal && (no_bytes < size_left))	// when malloc already have no_bytes available locallly, NO need to do brk() syscall
	{
		tmp = pt_mal;
		size_left -= no_bytes;
		pt_mal += no_bytes;

		return ((void *) tmp);		// return from local
	}

	div = (no_bytes / 4096);
	nu = (no_bytes % 4096);

	if (div)
	{
		final = (div * 4096);
	}
	
	if (nu)
	{
		final += 4096;
	}

	if ((tmp = (char*) __syscall1(MALLOC, final)) == NULL)
	{
		return NULL;
	}

	size_left = (final - no_bytes-100);

	pt_mal = (tmp + no_bytes);

	return ((void *) tmp);
}	


