#include <cstdint>
#include <cstdio>
extern "C" void alias(void*);
int main(){uint64_t value=0;alias(&value);if(value!=0x99aabbccddeeff00ULL)return 1;std::puts("aliased outputs: final assignment passed");}
