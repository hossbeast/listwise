#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rpath.h"

char * rpath(const char * path, char * resolved, size_t sz)
{
	size_t z = 0;

	if(path[0] == '/')
	{
		z += snprintf(resolved, sz - z, "/");
		path++;
	}

	while(path[0])
	{
		if(path[0] == '/')
		{
			path++;
		}
		else
		{
			const char * s = path;
			const char * e = s + 1;

			while(e[0] != 0 && e[0] != '/')
				e++;

			if(e - s == 1 && s[0] == '.')
			{
				// no op
			}
			else if(e - s == 2 && s[0] == '.' && s[1] == '.')
			{
				
			}
			else
			{
				
			}

			path = e;
		}
	}
}
