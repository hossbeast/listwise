#ifndef _LISTWISE_OBJECT_H
#define _LISTWISE_OBJECT_H

/* LISTWISE OBJECT API
**
**  typically liblistwise manipulates lists of strings. using this api it is possible
**  to also manipulate lists of objects of various types
**
**  first, use listwise_register_object to associate a unique type identifier with an
**  object definition, which is a list of function pointers for manipulating instances
**  of that object
**
**  use the lstack_* api's in listwise/object.h add instances of the object to an lstack
**
**  (see the a(apply) operator)
**
*/

typedef struct
{
	/// string
	//
	// SUMMARY
	// 	get string/length pointers which liblistwise uses to treat objects of
	//  this type as a string
	//
	// PARAMETERS
	//  o - pointer to the object
	//  s - pointer to string returned here
	//  l - pointer to length returned here
	//
	// RETURNS
	//  0 on error, 1 otherwise
	//
	int (*string)(void * o, char ** s, int * l);

	/// reflect
	//
	// SUMMARY
	//  access a property on an object of this type. usually this means replace the
	//  instance of the object with 1 or more other objects and/or strings
	//
	// PARAMETERS
	//  o        - pointer to the object
	//  property - property name
	//  r        - replacement objects/strings
	//  rtypes   - list of types of length rl
	//  rls      - list of lengths of length rl
	//  rl       - number of returned objects/strings
	//
	// RETURNS
	//  0 on error, 1 otherwise
	//
	int (*reflect)(void * o, char* property, void ** r, uint8_t * rtypes, int * rls, int * rl);

	/// destroy
	//
	// SUMMARY
	//  invoked just prior to freeing the object and forgetting the associated pointer
	//
	void (*destroy)(void* o);
} listwise_object;

///
/// [[ OBJECT API ]]
///

/// listwise_register
//
// SUMMARY
//  associate the specified type and object definition
// 
// PARAMETERS
//  type - unique user-defined object identifier
//  def  - object definition
// 
// RETURNS
//  0 on error (memory, io)
//
int listwise_register_object(uint8_t type, listwise_type* def);

///
/// [[ LSTACK API (for objects) ]]
///

/// lstack_obj_write
//
// place an object instance at x:y
//
int lstack_obj_write(lstack* const restrict ls, int x, int y, const void* const restrict o, uint8_t type)
	__attribute__((nonnull));

/// lstack_obj_add
//
// place an object instance at the next unused entry of list 0
//
int lstack_obj_add(lstack* const restrict ls, const void* const restrict o, uint8_t type)
	__attribute__((nonnull));

#endif
