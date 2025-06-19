#include "wrap.h"

int main() {
    fprintf(stdout, "Hello %s, your score is %d\n", "Alice", 95);
    fprintf(stderr, "This is an error: %s\n", "File not found");
    return 0;
}