#include <stdio.h>
int main() {
 int count = printf("%d %lld %.2f %.2Lf %s\n", -7, 0x100000002LL, 2.5, 0.25L, "wasm");
 return count == 29 ? 7 : 2;
}
