#if !defined(__OPTIMIZE__) || defined(__NO_INLINE__) || defined(__OPTIMIZE_SIZE__)
#error Optimized compilation must not inherit unoptimized host macros
#endif
#if defined(__CET__)
#error The host compiler's CET setting must not be advertised by this backend
#endif
int main() { return 0; }
