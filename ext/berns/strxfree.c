#include "strxfree.h"
#include "ruby.h"

void strxfree(char * memory) {
	return ruby_xfree(memory);
}
