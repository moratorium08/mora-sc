#include <string.h>
#include <stdlib.h>


char *dup_str(char *s) {
    int n = strlen(s) + 1;
    char *dest = (char *)malloc(n);
    strcpy(dest, s);
    return dest;
}
