#include <stdlib.h>
#include <string.h>

#include <listwise/operator.h>
#include <listwise/object.h>

#include "xmem.h"
#include "control.h"

/*

fx (fix) operator - set or unset the default string property for specified object type(s)

NO ARGUMENTS

*/

static int op_exec(operation*, lstack*, int**, int*);

operator op_desc = {
	  .optype				= LWOP_ARGS_CANHAVE
	, .op_exec			= op_exec
	, .desc					= "set or unset default string property for objects"
};

int op_exec(operation* o, lstack* ls, int** ovec, int* ovec_len)
{
	listwise_object ** list = 0;
	int list_len = 0;

	fatal(listwise_enumerate_objects, &list, &list_len);

	int x;
	for(x = 0; x < list_len; x++)
	{
		xfree(&list[x]->string_property);

		if(o->argsl)
		{
			list[x]->string_property = calloc(1, o->args[x]->l+1);
			memcpy(list[x]->string_property, o->args[x]->s, o->args[x]->l);
		}
	}

	return 1;
}
