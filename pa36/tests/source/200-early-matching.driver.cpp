#include <climits>
#include <cstdio>
extern "C" unsigned long update(unsigned long, unsigned long);
int main() {
    const unsigned long values[] = {0,1,127,128,0x123456789abcdefUL,ULONG_MAX};
    for (unsigned long left : values) for (unsigned long right : values)
        if (update(left,right) != (((left + 17) ^ right) + 8)) return 1;
    std::puts("early writes, read/write and named matching passed");
}
