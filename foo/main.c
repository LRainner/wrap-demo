#include <stdio.h>

int foo(int x);

int main() {
    int result = foo(42);
    fprintf(stdout, "Result: %d\n", result);
    return 0;
}