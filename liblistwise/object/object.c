#include <stddef.h>

#include "listwise/internal.h"

#include "coll.h"
#include "idx.h"

#include "control.h"

typedef char* charstar;

union object_registry_t object_registry = { { .size = sizeof(listwise_object*) } };

int API listwise_register_object(uint8_t type, listwise_object * def)
{
	def->type = type;

	// add this type to the collection
	fatal(coll_doubly_add, &object_registry.c, &def, 0);

	// reindex
	fatal(idx_mkindex
		, object_registry.e
		, object_registry.l
		, object_registry.z
		, offsetof(typeof(object_registry.e[0][0]), type)
		, sizeof(((typeof(object_registry.e[0][0])*)0)->type)
		, INDEX_UNIQUE | INDEX_NUMERIC | INDEX_DEREF
		, &object_registry.by_type
	);
}

charstar API lstack_string(lstack* const restrict ls, int x, int y)
{
	if(ls->s[x].s[y].type)
	{
		listwise_object* o = idx_lookup_val(object_registry.by_type, &ls->s[x].s[y].type, 0);

		if(o)
		{
			char * s = 0;
			int l = 0;

			o->string(ls->s[x].s[y].s, 0, &s, &l);

			return s;
		}

		return 0;
	}

	return ls->s[x].s[y].s;
}

charstar API lstack_getstring(lstack* const restrict ls, int x, int y, char ** r, int * rl)
{
	if(ls->s[x].s[y].type)
	{
		listwise_object* o = idx_lookup_val(object_registry.by_type, &ls->s[x].s[y].type, 0);

		if(o)
		{
			char * s = 0;
			int l = 0;

			o->string(ls->s[x].s[y].s, 0, r, rl);

			return s;
		}

		return 0;
	}

	*r  = ls->s[x].s[y].s;
	*rl = ls->s[x].s[y].l;

	return *r;
}
