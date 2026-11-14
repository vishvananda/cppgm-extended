// Source views must honor the same final mode flags as object compilation.
#ifndef __GXX_RTTI
#error unexpected language mode
#endif
#ifndef __cpp_rtti
#error unexpected language mode
#endif
#ifndef __EXCEPTIONS
#error unexpected language mode
#endif
#ifndef __cpp_exceptions
#error unexpected language mode
#endif
int main() { return 0; }
