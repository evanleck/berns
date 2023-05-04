#include "ruby.h"
#include "strxnew.h"

char * strxnew(size_t size) {
	char *string = ruby_xmalloc(size);

	return string;
}
