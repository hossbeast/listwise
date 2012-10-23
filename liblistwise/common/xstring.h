#ifndef _XSTRING_H
#define _XSTRING_H

/* wrappers around certain functions from string.h */

/// xstrcmp
//
// semantically equivalent to strcmp, except that this function accepts length values
// for the two strings, which need not be null-terminated.
//
// not suitable for comparing numbers by passing pointers to them (although this would
// work on a big-endian machine)
//
// parameters
//
// A    - first string
// alen - length of A, or 0 if length of A == strlen(A)
// B    - second string
// blen - length of B, or 0 if length of B == strlen(B)
//
// ** ACTUALLY, see strncmp and strnicmp for this functionality
//
int xstrcmp(char* A, int alen, char* B, int blen, int case_insensitive);

/// xstrstr
//
// locates the byte sequence { B, B + blen } within the byte sequence { A, A + alen }
//
// if case_insensitive is set, those bytes in A and B in the range { 'A', 'z' } are compared
// without respect to case
//
// returns a pointer offset from A where B was found, or 0
//
char* xstrstr(char* A, int alen, char* B, int blen, int case_insensitive);

/// xstrcatf
//
// calculates size required to vprintf, reallocates *s and appends the new string onto *s
//
// returns zero on malloc failure
//
#define xstrcatf_onfail 0
int xstrcatf(char** s, char* fmt, ...);

/// xstrcat
//
// reallocates *s1, appends s2 onto *s1
//
// returns zero on malloc failure
//
#define xstrcat_onfail 0
int xstrcat(char** s1, const char* s2);

/// xstrdup
//
// copies s2 to *s1, reallocating *s1 if necessary, deallocating it if it was already allocated
//
// returns zero on malloc failure
//
#define xstrdup_onfail 0
int xstrdup(char** s1, const char* s2);

/// xsprintf
//
// - free(*s)
// - allocate(*s)
// - sprintf(fmt, ...) -> *s
//
#define xsprintf_onfail 0
int xsprintf(char** s, char* fmt, ...);

/// strcmplist
//
// compares s to the list of strings(sl) of length sl_len
//
// returns a pointer to the first match within the list(sl) or NULL if there was no match
//
char* strcmplist(char *s, char **sl, int sl_len, int case_insensitive);

/// vstrcmplist
//
// compares s to the variable list of strings
//
// returns a pointer to the first match within the variable list or NULL if there was no match
//
char* vstrcmplist(char *s, int case_insensitive, ...);

#endif
