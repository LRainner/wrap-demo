#include <stdio.h>

int __real_foo(int x);  // forward declaration

int __wrap_foo(int x) {
    fprintf(stdout, "[WRAP foo] foo called with %d\n", x);
    return __real_foo(x);
}

int foo(int x) {
    return x * 2;
}