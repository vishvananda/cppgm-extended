#if defined(__PIC__) || defined(__pic__) || __NO_INLINE__ != 7
#error Explicit macro overrides must follow the driver defaults
#endif
int main() { return 0; }
