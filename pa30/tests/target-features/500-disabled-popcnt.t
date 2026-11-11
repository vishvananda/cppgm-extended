#if defined(__POPCNT__)
#error Incorrect SSE4.2 population-count feature
#endif
int main() { return __builtin_popcount(0x901U) != 3; }
