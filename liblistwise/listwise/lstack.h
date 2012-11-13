#ifndef _LISTWISE_LSTACKH
#define _LISTWISE_LSTACKH

#include <listwise/generator.h>

///
/// [[ LSTACK API ]]
///

// iterate the selected elements of the 0th list of the lstack
#define LSTACK_LOOP_ITER(ls, x, go)											\
	for(x = 0; x < (ls)->s[0].l; x++)											\
	{																											\
		int go = 1;																					\
		if(!(ls)->sel.all)																	\
		{																										\
			go = 0;																						\
			if((ls)->sel.sl > (x/8))													\
			{																									\
				go = ((ls)->sel.s[x/8] & (0x01 << (x%8)));			\
			}																									\
		}

#define LSTACK_LOOP_DONE	}

/// lstack_exec
//
// if *r == 0, create it as the empty list stack
//
// then, executes the generator against *r
//
// parameters
//  g      - generator
//  init   - initial items to write at top of list stack
//  initls - lenghts of items in init, 0 for strlen
//  initl  - number of items in init
//  r      - input/output list stack
//
int lstack_exec(generator* g, char** init, int* initls, int initl, lstack** r);

/// lstack_dump
//
// print a list-stack to stdout
//
void lstack_dump(lstack*);

/// lstack_reset
//
// reset (but do not deallocate) a list stack
//
// no-op with zero-valued parameter
//
void lstack_reset(lstack*);

#endif
