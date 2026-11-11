#include <stdlib.h>
int main() {
 unsigned char *first = static_cast<unsigned char*>(calloc(17, 1));
 if (!first) return 1;
 for (int i = 0; i < 17; ++i) { if (first[i] != 0) return 2; first[i] = i + 1; }
 unsigned char *second = static_cast<unsigned char*>(realloc(first, 70000));
 if (!second) return 3;
 for (int i = 0; i < 17; ++i) if (second[i] != i + 1) return 4;
 second[69999] = 9;
 unsigned char *third = static_cast<unsigned char*>(malloc(70000));
 if (!third) return 5;
 third[69999] = second[69999];
 int result = third[69999] == 9 ? 7 : 6;
 free(third); free(second);
 return result;
}
