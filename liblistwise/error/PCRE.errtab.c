#include "PCRE.errtab.h"
#define APIDATA
etable errtab_PCRE = {
   .tag = "PCRE"
 , .min = ERRMIN_PCRE
 , .max = ERRMAX_PCRE
 , .v =  (typeof(((etable*)0)->v[0])[]) {
#define _E(a, b, c) [ a + (ERRMIN_PCRE * -1) ] = { .name = #b, .desc = c },
ETABLE_PCRE
#undef _E
}};
etable * APIDATA perrtab_PCRE = &errtab_PCRE;
