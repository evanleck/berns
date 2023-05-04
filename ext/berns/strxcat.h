#ifndef STRXCAT_H
#define STRXCAT_H
#include <stddef.h>

/*
 * Concatenate an arbitrary number of char* into a destination. Copies at most
 * size characters into the destination.
 */
extern char * strxcat(char *destination, size_t size, ...);

#endif
