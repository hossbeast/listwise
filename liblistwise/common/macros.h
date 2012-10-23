#ifndef _MACROS_H
#define _MACROS_H

#define MIN(a,b)            \
 ({ typeof (a) _a = (a);    \
     typeof (b) _b = (b);   \
   _a > _b ? _b : _a; })

#endif
