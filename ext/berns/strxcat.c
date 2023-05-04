#include <stdarg.h>

#include "strxcat.h"
#include "strxcpy.h"

char * strxcat(char *destination, size_t size, ...) {
	va_list sources;
	va_start(sources, size);

	while(size > 1) {
		char *source = va_arg(sources, char*);
		size_t position = strxcpy(destination, source, size);
		destination += position;
		size -= position;
	}

	va_end(sources);

	return destination;
}
