#ifndef STRXNEW_H
#define STRXNEW_H
#include <stddef.h>

/*
 * Allocates a new string using Ruby's ruby_xmalloc. Should be freed by
 * strxfree.
 */
extern char * strxnew(size_t size);

#endif
