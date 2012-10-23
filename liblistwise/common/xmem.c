#include <stdlib.h>
#include <string.h>

#include "xmem.h"

int xmalloc(void* target, size_t size)
{
	void** t = ((void**)target);
	*t = calloc(size, 1);
	return *t ? 1 : 0;
}

int xrealloc(void* target, size_t es, size_t ec, size_t oec)
{
	void** t = ((void**)target);
	void* a = realloc(*t, es * ec);

	if(es * ec)
	{
		if(a)
		{
			*t = a;

			if((ec - oec) > 0)
				memset(((char*)*t) + (oec * es), 0, (ec - oec) * es);

			return 1;
		}

		return 0;
	}

	return 1;
}

void xfree(void* target)
{
	void** t = (void**)target;

	free(*t);
	*t = 0;
}
