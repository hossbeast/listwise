#ifndef _RPATH_H
#define _RPATH_H

#define restrict __restrict

#define RP_DOT			0
#define RP_DOTDOT		0
#define RP_SLASH		0
#define RP_SYM			1
#define RP_SYMMNT		2
#define RP_

/// rpath - man 3 realpath
//
// SUMMARY
//  convert a path into a more canonical representation by performing a subset
//  of path translation operations. when all operations are specified, the result
//  is a canonicalized absolute pathname, as returned by the realpath function
//
//  DOT			- resolve references to the "." directory
//  DOTDOT	- resolve references to the ".." directory
//  SLASH		- resolve extra "/" charactesr
//  SYM			- resolve symbolic links which do not cross mount points
//  SYMMNT	- resolve symbolic links which cross mount points
//
// PARAMETERS
//  path				- path to convert
//  [resolved]	- if nonzero : resolved path is written here, but no more thatn sz bytes are written
//                if zero    : sz is ignored, and rpath returns a pointer to malloc'd memory
//
// RETURNS
//  0 on error, otherwise pointer to the resolved path
//
char * rpath(const char * restrict path, char * restrict resolved, size_t sz)
	__attribute((nonnull(1)));

#undef restrict
#endif
