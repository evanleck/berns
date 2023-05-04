#include "ruby.h"
#include "strxresize.h"

char * strxresize(char * memory, size_t size) {
	return ruby_xrealloc(memory, size);
}
