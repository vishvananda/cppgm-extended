// Source views must honor the same final mode flags as object compilation.
#ifdef __GXX_RTTI
#error unexpected language mode
#endif
#ifdef __cpp_rtti
#error unexpected language mode
#endif
#ifdef __EXCEPTIONS
#error unexpected language mode
#endif
#ifdef __cpp_exceptions
#error unexpected language mode
#endif
int main() { return 0; }
