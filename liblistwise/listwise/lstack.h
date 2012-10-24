#ifndef _LISTWISE_LSTACKH
#define _LISTWISE_LSTACKH

///
/// [[ LSTACK API ]]
///

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
