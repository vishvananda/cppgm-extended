#include <stdio.h>
#include <stdlib.h>
int initialize() { puts("init"); return 23; }
int state = initialize();
void finish() { puts("fini"); }
int main() { if (state != 23 || atexit(finish) != 0) return 2; puts("main"); return 7; }
