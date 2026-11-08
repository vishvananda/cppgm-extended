#if !defined(__SSE3__) || defined(__SSSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#error SSE disabling must remove dependent capabilities
#endif
int main() { return 0; }
