#include <stdio.h>

int __real_fprintf(FILE *__stream, const char *__fmt, ...);
int __wrap_fprintf(FILE *stream, const char *format, ...);