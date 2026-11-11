#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
static_assert(sizeof(size_t) == 4 && sizeof(intptr_t) == 4, "target headers");
int main(int argc, char **argv) {
 if (argc != 2) return 2;
 puts(argv[1]);
 return 7;
}
