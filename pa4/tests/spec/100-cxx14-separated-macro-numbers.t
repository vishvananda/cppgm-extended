#define STRING(x) #x
#define JOIN(a,b) a ## b
#if 1'000 == 1000 && 0b10'01 == 9
STRING(1'000)
JOIN(0b10,01)
#else
#error separated numbers lost their value
#endif
__cplusplus
