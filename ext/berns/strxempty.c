#import "strxempty.h"
#import "strxnew.h"

char * strxempty() {
  char *str = strxnew(1);
  str[0] = '\0';

  return str;
}
