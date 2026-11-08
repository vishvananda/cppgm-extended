#if !defined(__SSE3__) || !defined(__SSSE3__) || !defined(__SSE4_1__) || !defined(__SSE4_2__)
#error Missing implied SSE features
#endif
int main() { return 0; }
